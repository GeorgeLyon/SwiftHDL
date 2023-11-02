#pragma once

// Includes that fix incomplete includes in CIRCT and MLIR
#include <mlir/IR/OpDefinition.h>

// Includes
#include <circt/Firtool/Firtool.h>

namespace mlir {
inline MLIRContext *MLIRContextCreate() {
  return new MLIRContext();
}
} // namespace mlir
