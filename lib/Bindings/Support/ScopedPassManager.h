#pragma once

#include "SwiftHDL/Bindings/Support/ScopedBuilder.h"
#include "SwiftHDL/Bindings/Support/ScopedPassManager.h"

#include "OpaquePointer.h"
#include "ReferenceCountedPointer.h"

#include <mlir/Pass/PassManager.h>

namespace SwiftHDL {

struct ScopedPassManager::Implementation
    : public ReferenceCountedPointer::Implementation,
      public OpaquePointerRepresentable<
          ScopedPassManager::Implementation,
          AnyOpaquePointerRepresentable::Kind::PassManager> {
  friend class ScopedPassManager;

  /**
   Scoped pass manager maintains a strong reference to a builder.
   */
  ScopedBuilder builder;

  /**
   A flag to indicate whether a pass has been added to the pipleine which may
   transform the IR. If this is the case, we need to invalidate the builder
   since old references to Operations or Values may no longer be valid.
   */
  bool shouldInvalidateBuilderAfterRun = false;

  mlir::PassManager passManager;

  Implementation(const ScopedBuilder &builder)
      : builder(builder), passManager(builder.getContext()) {}
};

} // namespace SwiftHDL
