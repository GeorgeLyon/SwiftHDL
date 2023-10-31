#pragma once
#include <circt/Firtool/Firtool.h>

namespace mlir {
inline MLIRContext *MLIRContextCreate() {
  return new MLIRContext();
}
} // namespace mlir
