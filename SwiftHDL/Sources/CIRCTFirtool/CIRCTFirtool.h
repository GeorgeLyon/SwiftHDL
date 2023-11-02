#pragma once
#include <mlir/IR/OpDefinition.h>
#include <circt/Firtool/Firtool.h>

namespace mlir {
inline MLIRContext *MLIRContextCreate() {
  return new MLIRContext();
}
} // namespace mlir
