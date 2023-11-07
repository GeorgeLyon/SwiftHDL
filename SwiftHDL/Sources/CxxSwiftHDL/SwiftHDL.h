#pragma once

#include <swift-hdl/SwiftHDL.h>

namespace mlir {
inline MLIRContext *MLIRContextCreate() {
  return new MLIRContext();
}
} // namespace mlir
