#ifndef RUYIJS_PAGE_HELPERS_H
#define RUYIJS_PAGE_HELPERS_H
#include "js/PropertyAndElement.h"
#include "js/RootingAPI.h"
#include "js/ValueArray.h"

namespace ruyijs {
// A native constructor's length/name keys are lazy: they resolve to the END of
// the shape when first read. The donor's generated bindings answer length and
// name before the permanent prototype key, so force the pair right after the
// function is created and before anything defines own keys on it.
inline bool LeadFunctionKeys(JSContext* cx, JS::HandleObject function) {
  JS::RootedValue resolved(cx);
  return JS_GetProperty(cx, function, "length", &resolved) &&
         JS_GetProperty(cx, function, "name", &resolved);
}

class PageHelpers final {
 public:
  PageHelpers(JSContext* cx, JS::HandleObject page);
  PageHelpers(const PageHelpers&) = delete;
  PageHelpers& operator=(const PageHelpers&) = delete;
  bool Call(const char* method, const JS::HandleValueArray& args, JS::MutableHandleValue result);
  bool Enabled() const;
  bool Member(JS::HandleObject function, const char* iface, JS::HandleString name, const char* type);
  bool Member(JS::HandleObject function, const char* iface, const char* name, const char* type);
  bool Interface(const char* iface, JS::HandleObject target);
  bool Object(const char* iface, JS::HandleObject target);
  bool Accessor(const char* iface, JS::HandleObject target, const char* name);
  bool Constructor(const char* iface, JS::HandleObject function);
 private:
  JSContext* cx_;
  JS::PersistentRootedObject page_, helpers_;
  bool Ensure();
};
}
#endif
