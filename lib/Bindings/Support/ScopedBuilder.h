#ifndef SWIFT_HDL_BINDINGS_SUPPORT_SCOPED_BUILDER_INTERNAL_H_
#define SWIFT_HDL_BINDINGS_SUPPORT_SCOPED_BUILDER_INTERNAL_H_

#include "SwiftHDL/Bindings/Support/ScopedBuilder.h"
#include "SwiftHDL/Bindings/Support/ScopedContext.h"

#include "OpaquePointer.h"
#include "ReferenceCountedPointer.h"
#include "ScopeID.h"

#include <mlir/IR/Builders.h>

namespace SwiftHDL {

struct ScopedBuilder::Implementation
    : public ReferenceCountedPointer::Implementation,
      public OpaquePointerRepresentable<
          ScopedBuilder::Implementation,
          AnyOpaquePointerRepresentable::Kind::Builder> {
  friend class ScopedBuilder;

  /**
   Builder IDs are allocated starting from the maximum value, decreasing by 1
   every time. This allows us to distinguish context and builder IDs visually
   when debugging, and makes it less likely for a collision to occur (though a
   collision would only matter if we accidentally reinterpreted a builder ID as
   a context ID).
   */
  struct BuilderID
      : detail::ScopeID<uint32_t, std::numeric_limits<uint32_t>::max(), -1> {
    using ScopeID::ScopeID;
    using ScopeID::operator==;
  };
  BuilderID id;

  bool isValid = true;
  ScopedContext context;
  mlir::OpBuilder builder;
  Implementation(const ScopedContext &context)
      : context(context), builder(context) {}

  struct ScopedContainer {
    BuilderID builderID;
    ScopedContainer(const ScopedBuilder &builder)
        : builderID(builder.get()->id) {}
    bool isValidInScope(const ScopedBuilder &builder) const {
      auto impl = builder.get();
      return impl->isValid && impl->id == builderID;
    }
  };

  struct OpStateContainer
      : OpaquePointerRepresentable<
            OpStateContainer, AnyOpaquePointerRepresentable::Kind::OpState>,
        ScopedContainer {
    mlir::OpState opState;
    OpStateContainer(const ScopedBuilder &builder, mlir::OpState opState)
        : ScopedContainer(builder), opState(opState) {}
  };
  struct RegionContainer
      : OpaquePointerRepresentable<RegionContainer,
                                   AnyOpaquePointerRepresentable::Kind::Region>,
        ScopedContainer {
    mlir::Region *region;
    RegionContainer(const ScopedBuilder &builder, mlir::Region *region)
        : ScopedContainer(builder), region(region) {}
  };
  struct BlockContainer
      : OpaquePointerRepresentable<BlockContainer,
                                   AnyOpaquePointerRepresentable::Kind::Block>,
        ScopedContainer {
    mlir::Block *block;
    BlockContainer(const ScopedBuilder &builder, mlir::Block *block)
        : ScopedContainer(builder), block(block) {}
  };
  struct ValueContainer
      : OpaquePointerRepresentable<ValueContainer,
                                   AnyOpaquePointerRepresentable::Kind::Value>,
        ScopedContainer {
    mlir::Value value;
    ValueContainer(const ScopedBuilder &builder, mlir::Value value)
        : ScopedContainer(builder), value(value) {}
  };
};

} // namespace SwiftHDL

#endif // SWIFT_HDL_BINDINGS_SUPPORT_SCOPED_BUILDER_INTERNAL_H_
