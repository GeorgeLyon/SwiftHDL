#pragma once

#include "SwiftHDL/Bindings/Support/ScopedContext.h"

#include "OpaquePointer.h"
#include "ReferenceCountedPointer.h"
#include "ScopeID.h"

#include <mlir/IR/MLIRContext.h>

namespace SwiftHDL {

struct ScopedContext::Implementation
    : public ReferenceCountedPointer::Implementation,
      public OpaquePointerRepresentable<
          ScopedContext::Implementation,
          AnyOpaquePointerRepresentable::Kind::Context> {
  friend class Context;

  /**
   Used for other types to get access to a Context's implementation
   */
  static Implementation *get(const ScopedContext &context) {
    return context.get();
  }

  struct ContextID
      : detail::ScopeID<uint32_t, std::numeric_limits<uint32_t>::min(), 1> {
    using ScopeID::ScopeID;
    using ScopeID::operator==;
  };
  ContextID id;

  mutable void *userData = nullptr;
  void (*userDataDeleter)(void *) = nullptr;

  ~Implementation() override {
    if (userDataDeleter)
      userDataDeleter(userData);
  }

  mutable mlir::MLIRContext mlirContext;

  struct ScopedContainer {
#ifdef SWIFT_HDL_BINDINGS_SUPPORT_SINGULAR_CONTEXT
    ContextScopedContainer() {}
    bool isValidInContext(const ScopedContext &context) const { return true; }
#else
    ContextID contextID;
    ScopedContainer(const ScopedContext &context)
        : contextID(ScopedContext::Implementation::get(context)->id) {}
    bool isValidInContext(const ScopedContext &context) const {
      return ScopedContext::Implementation::get(context)->id == contextID;
    }
#endif
  };

  struct AttributeContainer
      : OpaquePointerRepresentable<
            AttributeContainer, AnyOpaquePointerRepresentable::Kind::Attribute>,
        ScopedContainer {
    mlir::Attribute attribute;
    AttributeContainer(const ScopedContext &context, mlir::Attribute attribute)
        : ScopedContainer(context), attribute(attribute) {}
  };
  struct TypeContainer
      : OpaquePointerRepresentable<TypeContainer,
                                   AnyOpaquePointerRepresentable::Kind::Type>,
        ScopedContainer {
    mlir::Type type;
    TypeContainer(const ScopedContext &context, mlir::Type type)
        : ScopedContainer(context), type(type) {}
  };
};

} // namespace SwiftHDL
