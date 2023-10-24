#ifndef SWIFT_HDL_BINDINGS_SUPPORT_SCOPED_BUILDER_H_
#define SWIFT_HDL_BINDINGS_SUPPORT_SCOPED_BUILDER_H_

#include "SwiftHDL/Bindings/Support/OpaquePointer.h"
#include "SwiftHDL/Bindings/Support/ReferenceCountedPointer.h"
#include "SwiftHDL/Bindings/Support/ScopedContext.h"

#include <llvm/Support/Casting.h>
#include <mlir/IR/OpDefinition.h>
#include <type_traits>

namespace mlir {
class OpBuilder;
}

namespace SwiftHDL {

class ScopedBuilder : ReferenceCountedPointer {
public:
  // -- Lifecycle
  ScopedBuilder(const ScopedContext &context);

  /**
   Invalidates any Operations, Blocks, Regions and Values that were created
   using this builder. Should be called when we expect destructive changes to
   the IR.
   */
  void invalidate();

  // -- Conversion to/from OpaquePointer
  static ScopedBuilder getFromOpaquePointer(OpaquePointer);
  OpaquePointer toRetainedOpaquePointer();

  // -- Accessors
  ScopedContext getContext() const;
  mlir::OpBuilder *operator->();

  // -- Wrapping
  OpaquePointer wrap(const mlir::Type &) const;
  OpaquePointer wrap(const mlir::Attribute &) const;
  OpaquePointer wrap(const mlir::Location &) const;
  OpaquePointer wrap(const mlir::OpState &) const;
  OpaquePointer wrap(mlir::Block *) const;
  OpaquePointer wrap(mlir::Region *) const;
  OpaquePointer wrap(const mlir::Value &) const;

  // -- Unwrapping
  template <typename T> T unwrap(OpaquePointer opaquePointer) const {
    return Unwrapper<T>::unwrap(*this, opaquePointer);
  }

  // -- Reference Counted Pointer Internals
  struct Implementation;

private:
  // -- Reference Counted Pointer Internals
  Implementation *get() const;
  using ReferenceCountedPointer::ReferenceCountedPointer;

  // -- Unwrapping logic
  template <typename T, typename = void> struct Unwrapper {
    static T unwrap(const ScopedBuilder &builder, OpaquePointer opaquePointer) {
      // Fall back on context for other types
      return builder.getContext().unwrap<T>(opaquePointer);
    };
  };
  mlir::Value unwrapValue(OpaquePointer opaquePointer) const;
  template <typename T>
  struct Unwrapper<T, std::enable_if_t<std::is_base_of_v<mlir::Value, T>>> {
    static T unwrap(const ScopedBuilder &builder, OpaquePointer opaquePointer) {
      return llvm::cast<T>(builder.unwrapValue(opaquePointer));
    }
  };
  mlir::OpState unwrapOpState(OpaquePointer opaquePointer) const;
  template <typename T>
  struct Unwrapper<T, std::enable_if_t<std::is_base_of_v<mlir::OpState, T>>> {
    static T unwrap(const ScopedBuilder &builder, OpaquePointer opaquePointer) {
      return llvm::cast<T>(builder.unwrapOpState(opaquePointer));
    }
  };
  template <> struct Unwrapper<mlir::Operation *> {
    static mlir::Operation *unwrap(const ScopedBuilder &builder,
                                   OpaquePointer opaquePointer) {
      return builder.unwrapOpState(opaquePointer).getOperation();
    }
  };
  mlir::Block *unwrapBlock(OpaquePointer opaquePointer) const;
  template <> struct Unwrapper<mlir::Block *> {
    static mlir::Block *unwrap(const ScopedBuilder &builder,
                               OpaquePointer opaquePointer) {
      return builder.unwrapBlock(opaquePointer);
    }
  };
  mlir::Region *unwrapRegion(OpaquePointer opaquePointer) const;
  template <> struct Unwrapper<mlir::Region *> {
    static mlir::Region *unwrap(const ScopedBuilder &builder,
                                OpaquePointer opaquePointer) {
      return builder.unwrapRegion(opaquePointer);
    }
  };
};

} // namespace SwiftHDL

#endif // SWIFT_HDL_BINDINGS_SUPPORT_SCOPED_BUILDER_H_
