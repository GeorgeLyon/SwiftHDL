#pragma once

#include "SwiftHDL/Bindings/Support/OpaquePointer.h"
#include <cassert>

namespace SwiftHDL {

class AnyOpaquePointerRepresentable {
  friend class OpaquePointer;

public:
  /**
   Each `Kind` should correspond to exactly 1 concrete C++ type, this ensures
   that we can verify the type of an opaque pointer at runtime.

   We don't use `mlir::TypeID` for this because the concrete types are not MLIR
   types, and there is some fragility around dynamic linking and MLIR types
   being consistent.
   */
  enum class Kind {
    Attribute,
    Block,
    Builder,
    Context,
    OpState,
    PassManager,
    Region,
    Type,
    Value
  };
  template <typename Derived, Kind kind>
  friend class OpaquePointerRepresentable;

private:
  Kind _kind;
  explicit AnyOpaquePointerRepresentable(Kind kind) : _kind(kind) {}
  bool is(Kind other) const { return _kind == other; }
};

template <typename Derived, AnyOpaquePointerRepresentable::Kind kind>
class OpaquePointerRepresentable : public AnyOpaquePointerRepresentable {
public:
  OpaquePointerRepresentable() : AnyOpaquePointerRepresentable(kind) {}

  static Derived *getFromOpaquePointer(OpaquePointer pointer) {
    auto any = static_cast<AnyOpaquePointerRepresentable *>(pointer.get());
    assert(any->is(kind));
    return static_cast<Derived *>(any);
  }

  OpaquePointer toOpaquePointer() {
    return OpaquePointer(static_cast<AnyOpaquePointerRepresentable *>(this));
  }
};

} // namespace SwiftHDL
