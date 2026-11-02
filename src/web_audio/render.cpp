#include "web_audio/internal.h"
#include "js/Conversions.h"
#include "web_audio/node_fields.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "js/Object.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "js/experimental/TypedData.h"
#include "web_audio/render_graph.h"

// The offline render: the node graph walks once per 128-frame render quantum,
// every node runs its Gecko algorithm on the mixed input of its incoming edges,
// and the destination's output becomes the rendered AudioBuffer. The oscillator,
// the gain node and the dynamics compressor carry real DSP; the other nodes pass
// their input through until their own algorithm is ported (D34).
namespace ruyijs::web_audio {
namespace {

// Reads a node's AudioParam value through the one parameter table.
double ParamValue(State& state, JS::HandleObject node, int32_t index, double fallback) {
  JS::Value param = JS::GetReservedSlot(node, kNodeParamBase + index);
  if (!param.isObject()) return fallback;
  JS::Value id = JS::GetReservedSlot(&param.toObject(), 0);
  double value = fallback;
  if (id.isInt32() && id.toInt32() > 0 &&
      state.model.param_value(uint32_t(id.toInt32()), &value))
    return value;
  return fallback;
}

// OscillatorNode.type as the shape the band-limited tables generate.
WaveTables::Shape ShapeOf(JSContext* cx, JS::HandleObject node, bool* ok) {
  *ok = true;
  JS::Value value = JS::GetReservedSlot(node, kNodeTypeSlot);
  if (value.isString()) {
    JS::RootedString text(cx, value.toString());
    const struct {
      const char* name;
      WaveTables::Shape shape;
    } kShapes[] = {{"sine", WaveTables::kSine},
                   {"square", WaveTables::kSquare},
                   {"sawtooth", WaveTables::kSawtooth},
                   {"triangle", WaveTables::kTriangle}};
    for (const auto& candidate : kShapes) {
      bool matched = false;
      if (!JS_StringEqualsAscii(cx, text, candidate.name, &matched)) {
        *ok = false;
        return WaveTables::kSine;
      }
      if (matched) return candidate.shape;
    }
  }
  return WaveTables::kSine;  // "custom" stays a sine until PeriodicWave data exists
}

// Monaural output is duplicated into every input channel, a wider output is
// averaged down, and equal layouts add sample by sample.
void MixInto(const float* source, uint32_t source_channels, float* destination,
             uint32_t channels) {
  if (source_channels == channels) {
    for (int32_t i = 0; i < kRenderQuantum * int32_t(channels); ++i) destination[i] += source[i];
    return;
  }
  if (source_channels == 1) {
    for (int32_t i = 0; i < kRenderQuantum; ++i)
      for (uint32_t channel = 0; channel < channels; ++channel)
        destination[channel * kRenderQuantum + i] += source[i];
    return;
  }
  const float scale = 1.0f / float(source_channels);
  for (uint32_t channel = 0; channel < source_channels; ++channel)
    for (int32_t i = 0; i < kRenderQuantum; ++i)
      destination[i] += source[channel * kRenderQuantum + i] * scale;
}

// The AudioParam slots the buffer source reads: playbackRate's field holds
// either the AudioParam object or a plain number.
double PlaybackRateOf(State& state, JSContext* cx, JS::HandleObject node) {
  JS::RootedValue value(cx);
  if (!ReadField(cx, node, "playbackRate", &value)) return 1.0;
  if (value.isNumber()) return value.toNumber();
  if (value.isObject()) {
    // The field stores the AudioParam object: slot 0 is its model id, the
    // live value lives behind the model, exactly as ParamValue reads it.
    JS::Value id = JS::GetReservedSlot(&value.toObject(), 0);
    double live = 1.0;
    if (id.isInt32() && id.toInt32() > 0 &&
        state.model.param_value(uint32_t(id.toInt32()), &live))
      return live;
    return 1.0;
  }
  return 1.0;
}

// Runs one render quantum of |node|'s own algorithm.
bool ProcessNode(State& state, RenderNode& node, int32_t quantum_offset,
                 double sample_rate) {
  JSContext* cx = state.cx;
  if (node.kind == kBufferSourceNode) {
    JS::RootedObject object(cx, node.node);
    JS::RootedValue bufferValue(cx);
    if (!ReadField(cx, object, "buffer", &bufferValue)) return false;
    if (!bufferValue.isObject()) {  // no buffer yet: silence
      std::fill(node.output.begin(), node.output.end(), 0.0f);
      return true;
    }
    JS::RootedObject buffer(cx, &bufferValue.toObject());
    const int32_t length = JS::GetReservedSlot(buffer, kBufferLengthSlot).toInt32();
    const int32_t channels = JS::GetReservedSlot(buffer, kBufferCountSlot).toInt32();
    const double rate = JS::GetReservedSlot(buffer, kBufferRateSlot).toDouble();
    JS::Value data = JS::GetReservedSlot(buffer, kBufferDataSlot);
    if (length <= 0 || channels <= 0 || !data.isObject()) {
      std::fill(node.output.begin(), node.output.end(), 0.0f);
      return true;
    }
    JS::RootedObject schedule(cx, &data.toObject());
    JS::Value start = JS::GetReservedSlot(object, kNodeStartSlot);
    const double when = start.isNumber() ? start.toNumber() : 0.0;
    JS::Value offsetValue = JS::GetReservedSlot(object, kNodeOffsetSlot);
    const double offset_seconds = offsetValue.isNumber() ? offsetValue.toNumber() : 0.0;
    JS::Value stop = JS::GetReservedSlot(object, kNodeStopSlot);
    const double stop_when = stop.isNumber() ? stop.toNumber() : 1.0e9;
    JS::RootedValue loopValue(cx);
    bool loops = false;
    double loop_start = 0, loop_end = length;
    if (ReadField(cx, object, "loop", &loopValue)) loops = JS::ToBoolean(loopValue);
    if (ReadField(cx, object, "loopStart", &loopValue) && loopValue.isNumber())
      loop_start = std::max(0.0, loopValue.toNumber()) * rate;
    if (ReadField(cx, object, "loopEnd", &loopValue) && loopValue.isNumber() &&
        loopValue.toNumber() > 0)
      loop_end = std::min(loopValue.toNumber() * rate, double(length));
    const double playback = PlaybackRateOf(state, cx, object);
    const double ratio = (rate > 0 && sample_rate > 0) ? rate / sample_rate : 1.0;
    const double begin = when * sample_rate;
    const double end = stop_when * sample_rate;
    const double head = offset_seconds * rate;
    JS::RootedValue channelValue(cx);
    for (uint32_t channel = 0; channel < node.channels; ++channel) {
      const uint32_t source = std::min(channel, uint32_t(channels) - 1);
      float* out = &node.output[size_t(channel) * kRenderQuantum];
      if (!data.isObject() || !JS_GetElement(cx, schedule, source, &channelValue) ||
          !channelValue.isObject()) {
        for (int32_t i = 0; i < kRenderQuantum; ++i) out[i] = 0.0f;
        continue;
      }
      JS::RootedObject samples(cx, &channelValue.toObject());
      JS::RootedValue sample(cx);
      for (int32_t i = 0; i < kRenderQuantum; ++i) {
        const double timeline = double(quantum_offset + i);
        if (timeline < begin || timeline >= end) { out[i] = 0.0f; continue; }
        double position = head + (timeline - begin) * playback * ratio;
        if (loops && loop_end > loop_start &&
            position >= loop_end)
          position = loop_start + std::fmod(position - loop_start,
                                            loop_end - loop_start);
        const int32_t index = int32_t(position);
        if (index < 0 || index >= length) { out[i] = 0.0f; continue; }
        if (!JS_GetElement(cx, samples, uint32_t(index), &sample)) return false;
        out[i] = float(sample.isNumber() ? sample.toNumber() : 0.0);
      }
    }
    return true;
  }
  if (node.kind == kOscillatorNode) {
    node.voice.Compute(node.output.data(), kRenderQuantum);
    return true;
  }
  if (node.kind == kCompressorNode) {
    JS::RootedObject object(cx, node.node);
    CompressorParameters parameters;
    parameters.threshold = float(ParamValue(state, object, 0, parameters.threshold));
    parameters.knee = float(ParamValue(state, object, 1, parameters.knee));
    parameters.ratio = float(ParamValue(state, object, 2, parameters.ratio));
    parameters.attack = float(ParamValue(state, object, 3, parameters.attack));
    parameters.release = float(ParamValue(state, object, 4, parameters.release));
    std::vector<const float*> source(size_t(node.channels));
    std::vector<float*> target(size_t(node.channels));
    for (uint32_t channel = 0; channel < node.channels; ++channel) {
      source[channel] = &node.input[size_t(channel) * kRenderQuantum];
      target[channel] = &node.output[size_t(channel) * kRenderQuantum];
    }
    node.compressor.Process(source.data(), target.data(), node.channels, kRenderQuantum,
                            parameters);
    return true;
  }
  if (node.kind == kGainNode) {
    JS::RootedObject object(cx, node.node);
    const float gain = float(ParamValue(state, object, 0, 1.0));
    for (size_t i = 0; i < node.output.size(); ++i) node.output[i] = node.input[i] * gain;
    return true;
  }
  node.output = node.input;  // pass-through for the nodes without DSP yet
  return true;
}

}  // namespace

bool RenderPlaceholder(State& state, JS::HandleObject context,
                       JS::HandleObject buffer) {
  JSContext* cx = state.cx;
  const int32_t length = JS::GetReservedSlot(buffer, kBufferLengthSlot).toInt32();
  const int32_t channels = JS::GetReservedSlot(buffer, kBufferCountSlot).toInt32();
  const double rate = JS::GetReservedSlot(buffer, kBufferRateSlot).toDouble();
  if (length <= 0 || channels <= 0) return true;
  std::vector<RenderNode> nodes;
  if (!BuildRenderNodes(state, context, float(rate), &nodes)) return false;
  if (nodes.empty()) return true;  // no source: the buffer stays silent
  for (RenderNode& node : nodes) {
    if (node.kind != kOscillatorNode) continue;
    JS::RootedObject object(cx, node.node);
    bool ok = true;
    const WaveTables::Shape shape = ShapeOf(cx, object, &ok);
    if (!ok) return false;
    node.voice.Configure(float(rate), shape);
    node.voice.SetFrequency(float(ParamValue(state, object, 0, 440.0)));
  }
  const JS::Value destination_value = JS::GetReservedSlot(context, kContextDestinationSlot);
  const RenderNode* destination_node = nullptr;
  for (const RenderNode& node : nodes)
    if (destination_value.isObject() && node.node == &destination_value.toObject())
      destination_node = &node;
  if (!destination_node) return true;  // a graph without a destination renders silence
  std::vector<std::vector<float>> rendered(static_cast<size_t>(channels));
  for (auto& channel : rendered) channel.assign(size_t(length), 0.0f);
  for (int32_t offset = 0; offset < length; offset += kRenderQuantum) {
    for (size_t index = 0; index < nodes.size(); ++index) {
      RenderNode& node = nodes[index];
      std::fill(node.input.begin(), node.input.end(), 0.0f);
      for (size_t other : node.inputs)
        MixInto(nodes[other].output.data(), nodes[other].channels, node.input.data(),
                node.channels);
      if (!ProcessNode(state, node, offset, rate)) return false;
    }
    // The destination's output is the rendered quantum. Gecko clips at the
    // device, so an offline render keeps values outside [-1, 1].
    const RenderNode& last = *destination_node;
    const int32_t frames = std::min(kRenderQuantum, length - offset);
    for (int32_t channel = 0; channel < channels; ++channel) {
      const uint32_t source = std::min(uint32_t(channel), last.channels - 1);
      const float* samples = &last.output[size_t(source) * kRenderQuantum];
      std::copy(samples, samples + frames, rendered[size_t(channel)].begin() + offset);
    }
  }
  JS::RootedObject data(cx, &JS::GetReservedSlot(buffer, kBufferDataSlot).toObject());
  for (int32_t channel = 0; channel < channels; ++channel) {
    JS::RootedValue element(cx);
    if (!JS_GetElement(cx, data, uint32_t(channel), &element)) return false;
    JS::RootedObject array(cx, &element.toObject());
    size_t array_length = 0;
    bool shared = false;
    float* samples = nullptr;
    if (!JS_GetObjectAsFloat32Array(array.get(), &array_length, &shared, &samples) ||
        !samples)
      return WebKit::Fail(cx, "AudioBuffer channel cache is not a Float32Array");
    const size_t limit = std::min(array_length, rendered[size_t(channel)].size());
    std::copy(rendered[size_t(channel)].begin(), rendered[size_t(channel)].begin() + limit,
              samples);
  }
  return true;
}

}  // namespace ruyijs::web_audio
