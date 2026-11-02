#include "runtime/scheduling_control.h"
#include "runtime/navigation_replay.h"
#include "runtime/environment_control.h"
#include "runtime/runtime.h"
#include "jsapi.h"
#include "js/PropertyAndElement.h"
#include "js/Conversions.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <thread>

namespace ruyijs {
using execution_scheduling::Error;
// Debug hang probe: a task that never yields spins inside one JS frame; the
// interrupt callback then runs in that frame and records its caller.
namespace {
std::atomic<JSContext*> gHangCx{nullptr};
std::atomic<long> gPetted{0};
std::atomic<bool> gArmed{false};

bool HangProbe(JSContext* cx) {
  JS::AutoFilename file;
  uint32_t line = 0;
  JS::ColumnNumberOneOrigin column(0);
  const bool scripted = JS::DescribeScriptedCaller(&file, cx, &line, &column);
  if (FILE* dbg = fopen("/tmp/probe/hang_stack.txt", "a")) {
    fprintf(dbg, "[hang] scripted=%d caller=%s:%u:%u\n", scripted ? 1 : 0,
            scripted && file.get() ? file.get() : "(none)", line,
            column.oneOriginValue());
    fclose(dbg);
  }
  gPetted = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return true;
}

void ArmHangWatchdog(JSContext* cx) {
  static bool installed = false;
  if (!installed) {
    installed = true;
    gHangCx = cx;
    JS_AddInterruptCallback(cx, HangProbe);
    std::thread([] {
      for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!gArmed.load()) continue;
        const long now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (now - gPetted.load() >= 8) {
          if (auto* context = gHangCx.load()) JS_RequestInterruptCallback(context);
        }
      }
    }).detach();
  }
  gPetted = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  gArmed = true;
}
}
static bool Fields(JSContext* cx, JS::HandleObject request,
                    std::initializer_list<const char*> permitted) {
  JS::Rooted<JS::IdVector> keys(cx, JS::IdVector(cx));
  if (!JS_Enumerate(cx, request, &keys)) return false;
  for (size_t i = 0; i < keys.length(); ++i) {
    JS::RootedId id(cx, keys[i]);
    JS::RootedValue value(cx);
    if (!JS_IdToValue(cx, id, &value)) return false;
    JS::RootedString key(cx, JS::ToString(cx, value));
    std::string name;
    if (!key || !ToUTF8(cx, key, &name)) return false;
    if (name == "id" || name == "op" || name == "action") continue;
    bool found = false;
    for (auto field : permitted) if (name == field) { found = true; break; }
    if (!found) return Error(cx, "Unknown event_loop field");
  }
  return true;
}
static bool Number(JSContext* cx, JS::HandleObject request, const char* key,
                   double* number, bool required = false) {
  bool present;
  if (!JS_HasOwnProperty(cx, request, key, &present)) return false;
  if (!present) return !required || Error(cx, "Missing event_loop numeric field");
  JS::RootedValue value(cx);
  if (!JS_GetProperty(cx, request, key, &value)) return false;
  if (!value.isNumber() || !std::isfinite(value.toNumber()))
    return Error(cx, "event_loop field must be a finite number");
  *number = value.toNumber();
  return true;
}
bool SchedulingControl::Describe(const execution_scheduling::RunResult& value,
                                 JS::MutableHandleValue result) {
  JS::RootedObject object(cx_, JS_NewPlainObject(cx_));
  JS::RootedString mode(cx_, JS_NewStringCopyZ(cx_, value.state.logical ? "logical" : "system"));
  JS::RootedValue next(cx_, value.state.has_next ? JS::DoubleValue(value.state.next) : JS::NullValue());
  JS::RootedValue exhausted(cx_, JS::BooleanValue(value.exhausted));
  if (!object || !mode || !JS_DefineProperty(cx_, object, "mode", mode, JSPROP_ENUMERATE) ||
      !JS_DefineProperty(cx_, object, "now_ms", value.state.now, JSPROP_ENUMERATE) ||
      !JS_DefineProperty(cx_, object, "epoch_ms", value.state.epoch, JSPROP_ENUMERATE) ||
      !JS_DefineProperty(cx_, object, "pending", double(value.state.pending), JSPROP_ENUMERATE) ||
      !JS_DefineProperty(cx_, object, "next_due_ms", next, JSPROP_ENUMERATE) ||
      !JS_DefineProperty(cx_, object, "executed", value.executed, JSPROP_ENUMERATE) ||
      !JS_DefineProperty(cx_, object, "exhausted", exhausted, JSPROP_ENUMERATE)) return false;
  result.setObject(*object);
  return true;
}
bool SchedulingControl::Request(JS::HandleObject request, JS::MutableHandleValue result) {
  std::string action;
  if (!StringProperty(cx_, request, "action", &action)) return false;
  if (action == "configure_timer_replay" || action == "pending_timers" || action == "run_timer")
    return TimerSelection(request, action, result);
  if (action == "configure_message_replay" || action == "pending_messages" || action == "run_message")
    return MessageSelection(request, action, result);
  if (action == "configure_navigation_replay" || action == "navigation_replay_status") {
    return NavigationReplayRequest(cx_, owner_, request, action == "configure_navigation_replay",
                                   environment_.Frozen(), result);
  }
  if (action == "status") {
    return Fields(cx_, request, {}) && Describe({owner_.Inspect()}, result);
  }
  if (action == "configure") {
    std::string mode;
    if (!Fields(cx_, request, {"mode", "epoch_ms"}) ||
        !StringProperty(cx_, request, "mode", &mode)) return false;
    if (mode != "logical" && mode != "system") return Error(cx_, "Unknown scheduling clock mode");
    double epoch = 0;
    if (mode == "logical") {
      if (!Number(cx_, request, "epoch_ms", &epoch, true)) return false;
      if (std::trunc(epoch) != epoch || std::abs(epoch) > 8.64e12)
        return Error(cx_, "Logical epoch must be an integer in the exact millisecond range", true);
    } else {
      bool present;
      if (!JS_HasOwnProperty(cx_, request, "epoch_ms", &present)) return false;
      if (present) return Error(cx_, "System clock does not accept an epoch override");
    }
    return owner_.Configure(mode == "logical", epoch) && Describe({owner_.Inspect()}, result);
  }
  if (action != "advance" && action != "drain") return Error(cx_, "Unknown event_loop action");
  const bool advance = action == "advance";
  if (advance ? !Fields(cx_, request, {"ms", "step_ms", "max_tasks"}) :
                !Fields(cx_, request, {"max_tasks"})) return false;
  double ms = 0, step = 16.667, budget = 100000;
  if ((advance && (!Number(cx_, request, "ms", &ms, true) ||
                   !Number(cx_, request, "step_ms", &step))) ||
      !Number(cx_, request, "max_tasks", &budget)) return false;
  if (std::trunc(budget) != budget || budget < 1 || budget > 100000)
    return Error(cx_, "Task budget must be an integer from 1 to 100000", true);
  if (!owner_.ValidateAdvance(ms, step, uint32_t(budget))) return false;
  environment_.Freeze();
  owner_.Freeze();
  ArmHangWatchdog(cx_);
  const auto status = advance ? owner_.Advance(ms, step, uint32_t(budget)) : owner_.Drain(uint32_t(budget));
  gArmed = false;
  return Describe(status, result);
}
}
