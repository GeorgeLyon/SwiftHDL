#pragma once

#include "SwiftHDL/Bindings/Support/OpaquePointer.h"
#include "SwiftHDL/Bindings/Support/ReferenceCountedPointer.h"

namespace mlir {
struct LogicalResult;
class PassManager;
class Operation;
} // namespace mlir

namespace SwiftHDL {

class ScopedBuilder;

class ScopedPassManager : ReferenceCountedPointer {
public:
  explicit ScopedPassManager(const ScopedBuilder &builder);

  // -- Accessors

  mlir::PassManager *operator->();

  mlir::PassManager &getPassManager() { return *operator->(); }
  ScopedBuilder getBuilder() const;

  void setShouldInvalidateBuilderAfterRun(bool flag = true);

  /**
   Should be used in place of `PassManager::run` to ensure that the builder is
   invalidated if needed.
   */
  mlir::LogicalResult run(mlir::Operation *op);

  // -- Conversion to/from OpaquePointer

  static ScopedPassManager getFromOpaquePointer(OpaquePointer);
  OpaquePointer toRetainedOpaquePointer();

  // -- ReferenceCountedPointer Internals

  struct Implementation;

private:
  Implementation *get() const;
  using ReferenceCountedPointer::ReferenceCountedPointer;
};

} // namespace SwiftHDL
