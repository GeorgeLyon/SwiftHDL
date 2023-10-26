#pragma once

#include <circt/Dialect/FIRRTL/FIRRTLDialect.h>

namespace circt {
namespace firrtl {
inline void loadFIRRTLDialect(mlir::MLIRContext *context) {
  context->getLoadedDialect<circt::firrtl::FIRRTLDialect>();
}
}  
} // namespace circt
