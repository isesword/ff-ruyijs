#include "document_model/state.h"
#include "domain/document_model/foreign_element_interfaces.h"

namespace ruyijs::document_model {
bool InstallForeignElementInterfaces(State& state) {
  JS::RootedObject parent(state.cx);
  for (const auto& row : ruyijs::domain::document_model::kSvgElementInterfaces) {
    parent = state.Find(row.parent)->prototype;
    if (!state.DefineInterface(row.name, parent, Illegal)) return false;
  }
  // SVGSVGElement's zoomAndPan constants live on the interface object, in the
  // donor binding's sConstants order.
  {
    static const struct { const char* name; int32_t value; } zoom[] = {
      {"SVG_ZOOMANDPAN_UNKNOWN", 0}, {"SVG_ZOOMANDPAN_DISABLE", 1},
      {"SVG_ZOOMANDPAN_MAGNIFY", 2}};
    JS::RootedObject svgConstructor(state.cx, state.Find("SVGSVGElement")->constructor);
    const unsigned flags = JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT;
    for (const auto& entry : zoom) {
      JS::RootedValue number(state.cx, JS::Int32Value(entry.value));
      if (!JS_DefineProperty(state.cx, svgConstructor, entry.name, number, flags))
        return false;
    }
  }
  parent = state.Find("Element")->prototype;
  return state.DefineInterface("MathMLElement", parent, Illegal);
}
}
