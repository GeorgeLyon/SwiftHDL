//===-- circt-c/Dialect/Comb.h - C API for Comb dialect -----------*- C -*-===//
//
// This header declares the C interface for registering and accessing the
// Comb dialect. A dialect should be registered with a context to make it
// available to users of the context. These users must load the dialect
// before using any of its attributes, operations or types. Parser and pass
// manager can load registered dialects automatically.
//
//===----------------------------------------------------------------------===//

#ifndef CIRCT_C_DIALECT_COMB_H
#define CIRCT_C_DIALECT_COMB_H

#include "mlir-c/IR.h"

#ifdef __cplusplus
extern "C" {
#endif

MLIR_DECLARE_CAPI_DIALECT_REGISTRATION(Combinational, comb);

#ifdef __cplusplus
}
#endif

#endif // CIRCT_C_DIALECT_COMB_H
