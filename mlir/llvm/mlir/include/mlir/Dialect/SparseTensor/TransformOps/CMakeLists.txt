set(LLVM_TARGET_DEFINITIONS SparseTensorTransformOps.td)
mlir_tablegen(SparseTensorTransformOps.h.inc -gen-op-decls)
mlir_tablegen(SparseTensorTransformOps.cpp.inc -gen-op-defs)
add_public_tablegen_target(MLIRSparseTensorTransformOpsIncGen)
