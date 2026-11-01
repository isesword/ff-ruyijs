#pragma once

#include <cmath>
#include <cstring>
#include <vector>

#include "domain/audio/split_radix_fft.h"

// PeriodicWave::createBandLimitedTables (dom/media/webaudio/blink/PeriodicWave.cpp)
// driven through FFTBlock::GetInverse: the inverse real DFT of a PeriodicWave's
// Fourier coefficients, i.e. ffvpx's AV_TX_FLOAT_RDFT inverse at FFTBlock's unit
// inverse scaling.
//
// The macOS arm64 build links the frozen ffvpx tx sources themselves (see
// vendor/ffvpx-tx-native/): FF dispatches that platform to the hand-written
// NEON split-radix codelet, whose rounding no portable emulation reproduces
// bit-exactly, so the real transform is the only faithful one. Every other
// platform keeps the fold + portable split-radix emulation below, which
// matches the x86_64 fma3 codelet Gecko runs there.
#if defined(__aarch64__) && defined(__APPLE__)
#define RUYIJS_AUDIO_FFPX_TX 1
extern "C" {
// Declarations mirror libavutil/tx.h without pulling the LGPL header into
// this include graph.
typedef struct AVTXContext AVTXContext;
typedef void (*av_tx_fn)(AVTXContext*, void*, void*, long);
int av_tx_init(AVTXContext**, av_tx_fn*, int type, int inv, int len,
               const void* scale, unsigned long long flags);
void av_tx_uninit(AVTXContext**);
}
#endif

namespace ruyijs::domain::audio {

namespace wave_detail {

// MULT(fact[i], x) from ff_tx_rdft_init's table for scale 1.0f.
inline float Scaled(float factor, float value) { return factor * value; }

}  // namespace wave_detail

// The tx_template.c DECL_RDFT(c2r) pre-transform fold: the conjugate-
// symmetric half-complex spectrum in |data|[0..size/2] is rewritten, in
// place, into the packed form the split-radix inverse consumes. The
// fact[0..7] multipliers are m*(+0.5, +0.5, +1, -1, +0.5, -0.5, -1, +1)
// with m = 2 for the unit inverse scaling.
inline void RdftC2rFold(SpectrumValue* data, unsigned size) {
  const unsigned half = size / 2;
  const unsigned quarter = size / 4;
  data[0].im = data[half].re;
  const float dc = data[0].re;
  data[0].re = wave_detail::Scaled(1.0f, dc + data[0].im);
  data[0].im = wave_detail::Scaled(1.0f, dc - data[0].im);
  data[quarter].re = wave_detail::Scaled(2.0f, data[quarter].re);
  data[quarter].im = wave_detail::Scaled(-2.0f, data[quarter].im);

  const double step = 2.0 * 3.14159265358979323846 / double(size);
  for (unsigned i = 1; i < quarter; ++i) {
    const float tcos = float(std::cos(double(i) * step));
    const float tsin = float(std::cos(double(quarter - i) * step));
    const SpectrumValue low = data[i];
    const SpectrumValue high = data[half - i];
    const float t0_re = wave_detail::Scaled(1.0f, low.re + high.re);
    const float t0_im = wave_detail::Scaled(-1.0f, low.im - high.im);
    const float t1_re = wave_detail::Scaled(-1.0f, low.im + high.im);
    const float t1_im = wave_detail::Scaled(1.0f, low.re - high.re);
    float t2_re, t2_im;
    fft_detail::ComplexMultiply(&t2_re, &t2_im, t1_re, t1_im, tcos, tsin,
                                fft_detail::kFuseNone);
    data[i].re = t0_re + t2_re;
    data[i].im = t2_im - t0_im;
    data[half - i].re = t0_re - t2_re;
    data[half - i].im = t2_im + t0_im;
  }
}


// Fills |table| (|size| samples) with the band-limited waveform of the |real| /
// |imag| Fourier coefficients, keeping partials 1..|partials| and dropping the
// DC term exactly as createBandLimitedTables does before normalisation.
inline void BandLimitedWave(const float* real, const float* imag, unsigned partials,
                            unsigned size, float* table) {
  using wave_detail::Scaled;
  const unsigned half = size / 2;
  const unsigned quarter = size / 4;
  partials = partials < half ? partials : half - 1;

  // The coefficients are copied with a complex conjugate, and bins above the
  // band limit stay zero; then DC is cleared (it has no effect either way).
  std::vector<SpectrumValue> spectrum(half + 1);
  for (unsigned i = 1; i <= partials; ++i) {
    spectrum[i].re = real[i];
    spectrum[i].im = -imag[i];
  }
  spectrum[0].re = 0.0f;
  spectrum[0].im = 0.0f;
  spectrum[half].re = 0.0f;
  spectrum[half].im = 0.0f;

#ifdef RUYIJS_AUDIO_FFPX_TX
  // The real ffvpx inverse RDFT, driven exactly as FFTBlock::GetInverse
  // drives it (unit inverse scaling, complex-element stride).
  AVTXContext* ctx = nullptr;
  av_tx_fn fn = nullptr;
  float scale = 1.0f;
  if (av_tx_init(&ctx, &fn, 6 /* AV_TX_FLOAT_RDFT */, 1, int(size), &scale, 0) == 0) {
    fn(ctx, table, spectrum.data(), sizeof(float));
    av_tx_uninit(&ctx);
    return;
  }
#endif

  // ff_tx_rdft_c2r for inv == 1: the last bin's real part is the DC imaginary
  // part, DC becomes re/im = (re + im), (re - im), and both are scaled by 1.
  spectrum[0].im = spectrum[half].re;
  RdftC2rFold(spectrum.data(), size);

  std::vector<SpectrumValue> out(half);
  InverseComplexFft(spectrum.data(), out.data(), half);
  static_assert(sizeof(SpectrumValue) == 2 * sizeof(float),
                "the time-domain table is a flat float array");
  std::memcpy(table, out.data(), size_t(size) * sizeof(float));
}

}  // namespace ruyijs::domain::audio
