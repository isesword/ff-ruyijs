// Window-exposed interfaces the mac port had not installed: presence, donor
// member names and own-key order for Serial, SerialPort, ViewTimeline and
// ScrollTimeline. No implementation lives behind them — a member call answers
// a TypeError where Firefox resolves a promise, so only enumeration and
// attribute presence are faithful (the challenge surface sweeps names).
#include "browser_environment/state.h"
#include "runtime/web_kit.h"
#include "runtime/page_helpers.h"
#include "js/PropertyAndElement.h"
#include "js/Realm.h"

namespace ruyijs::browser_environment {
namespace {
const JSClass kPlainClass = {"Object", 0};

bool Unavailable(JSContext* cx, unsigned, JS::Value*) {
  return WebKit::FailKind(cx, "this interface is not available", JSEXN_TYPEERR);
}

JSObject* Method(JSContext* cx, JSNative native, const char* name, unsigned length) {
  JSFunction* function = JS_NewFunction(cx, native, length, 0, name);
  return function ? JS_GetFunctionObject(function) : nullptr;
}

// One interface object over a prototype of named members, in the donor's
// order: length, name, prototype lead, then the prototype members.
bool DefineStub(JSContext* cx, JS::HandleObject page, PageHelpers& helpers,
                const char* name, JS::HandleObject parent_prototype,
                const JSFunctionSpec* methods, const JSPropertySpec* accessors,
                JS::MutableHandleObject prototype, JS::MutableHandleObject function) {
  // A null parent chains to %Object.prototype% (JS_InitClass's nullptr
  // semantics); JS_NewObjectWithGivenProto with a null handle would leave the
  // prototype with a null [[Prototype]] instead.
  JS::RootedObject proto_parent(cx, parent_prototype);
  if (!proto_parent) {
    proto_parent = JS::GetRealmObjectPrototype(cx);
    if (!proto_parent) return false;
  }
  prototype.set(JS_NewObjectWithGivenProto(cx, &kPlainClass, proto_parent));
  JS::RootedFunction created(cx, JS_NewFunction(cx, Unavailable, 0, JSFUN_CONSTRUCTOR, name));
  JS::RootedObject constructor(cx, created ? JS_GetFunctionObject(created) : nullptr);
  if (!prototype || !constructor || !LeadFunctionKeys(cx, constructor) ||
      !JS_DefineProperty(cx, constructor, "prototype", prototype,
                         JSPROP_READONLY | JSPROP_PERMANENT) ||
      !JS_DefineProperty(cx, prototype, "constructor", constructor, 0) ||
      (methods && !JS_DefineFunctions(cx, prototype, methods)) ||
      (accessors && !JS_DefineProperties(cx, prototype, accessors)) ||
      !JS_DefineProperty(cx, page, name, constructor, 0) ||
      !helpers.Object(name, prototype) || !helpers.Constructor(name, constructor))
    return false;
  function.set(constructor);
  JS::RootedValue member(cx);
  JS::RootedObject memberFn(cx);
  for (const JSFunctionSpec* method = methods; method && method->name.string(); ++method) {
    if (!JS_GetProperty(cx, prototype, method->name.string(), &member) || !member.isObject())
      return false;
    memberFn = &member.toObject();
    if (!helpers.Member(memberFn, name, method->name.string(), "call")) return false;
  }
  return true;
}
}  // namespace

bool InstallWindowExtras(State& state) {
  JSContext* cx = state.cx;
  JS::RootedObject page(cx, state.page);
  JS::RootedObject eventTargetPrototype(cx);
  {
    JS::RootedValue target(cx);
    JS::RootedObject constructor(cx);
    if (!JS_GetProperty(cx, page, "EventTarget", &target) || !target.isObject())
      return false;
    constructor = &target.toObject();
    if (!JS_GetProperty(cx, constructor, "prototype", &target) || !target.isObject())
      return false;
    eventTargetPrototype = &target.toObject();
  }
  static const JSFunctionSpec serialMethods[] = {
    JS_FN("getPorts", Unavailable, 0, JSPROP_ENUMERATE),
    JS_FN("requestPort", Unavailable, 0, JSPROP_ENUMERATE), JS_FS_END};
  static const JSPropertySpec serialAccessors[] = {
    JS_PSGS("onconnect", Unavailable, Unavailable, JSPROP_ENUMERATE),
    JS_PSGS("ondisconnect", Unavailable, Unavailable, JSPROP_ENUMERATE), JS_PS_END};
  static const JSFunctionSpec serialPortMethods[] = {
    JS_FN("getInfo", Unavailable, 0, JSPROP_ENUMERATE),
    JS_FN("open", Unavailable, 1, JSPROP_ENUMERATE),
    JS_FN("setSignals", Unavailable, 0, JSPROP_ENUMERATE),
    JS_FN("getSignals", Unavailable, 0, JSPROP_ENUMERATE),
    JS_FN("close", Unavailable, 0, JSPROP_ENUMERATE),
    JS_FN("forget", Unavailable, 0, JSPROP_ENUMERATE), JS_FS_END};
  static const JSPropertySpec serialPortAccessors[] = {
    JS_PSGS("onconnect", Unavailable, Unavailable, JSPROP_ENUMERATE),
    JS_PSGS("ondisconnect", Unavailable, Unavailable, JSPROP_ENUMERATE),
    JS_PSG("connected", Unavailable, JSPROP_ENUMERATE),
    JS_PSG("readable", Unavailable, JSPROP_ENUMERATE),
    JS_PSG("writable", Unavailable, JSPROP_ENUMERATE), JS_PS_END};
  static const JSPropertySpec viewTimelineAccessors[] = {
    JS_PSG("subject", Unavailable, JSPROP_ENUMERATE),
    JS_PSG("startOffset", Unavailable, JSPROP_ENUMERATE),
    JS_PSG("endOffset", Unavailable, JSPROP_ENUMERATE), JS_PS_END};
  static const JSPropertySpec scrollTimelineAccessors[] = {
    JS_PSG("source", Unavailable, JSPROP_ENUMERATE),
    JS_PSG("axis", Unavailable, JSPROP_ENUMERATE), JS_PS_END};
  JS::RootedObject scrollTimelinePrototype(cx), viewTimelinePrototype(cx),
      unusedFunction(cx);
  return DefineStub(cx, page, *state.helpers, "Serial", eventTargetPrototype,
                    serialMethods, serialAccessors, &scrollTimelinePrototype,
                    &unusedFunction) &&
         DefineStub(cx, page, *state.helpers, "SerialPort", eventTargetPrototype,
                    serialPortMethods, serialPortAccessors, &scrollTimelinePrototype,
                    &unusedFunction) &&
         DefineStub(cx, page, *state.helpers, "ScrollTimeline", nullptr, nullptr,
                    scrollTimelineAccessors, &scrollTimelinePrototype,
                    &unusedFunction) &&
         DefineStub(cx, page, *state.helpers, "ViewTimeline", scrollTimelinePrototype,
                    nullptr, viewTimelineAccessors, &viewTimelinePrototype,
                    &unusedFunction);
}  // namespace ruyijs::browser_environment
}
