#ifndef RUYIJS_RUNTIME_SURFACE_PARITY_H
#define RUYIJS_RUNTIME_SURFACE_PARITY_H
#include "jsapi.h"

namespace ruyijs {
bool InstallSurfaceParity(JSContext* cx, JS::HandleObject page);
}
#endif
