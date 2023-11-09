#pragma once

#include <swift-hdl/SwiftHDL.h>

namespace mlir {
/**
 Creates an MLIRContext
 - note: There is no way to write `MLIRContext` in Swift because it is an immovable type, so we need to make a utility method
 */
inline MLIRContext *MLIRContextCreate() {
  return new MLIRContext();
}
} // namespace mlir
