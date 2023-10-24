#ifndef SWIFT_HDL_BINDINGS_SUPPORT_OPAQUE_POINTER_H_
#define SWIFT_HDL_BINDINGS_SUPPORT_OPAQUE_POINTER_H_

namespace SwiftHDL {

class OpaquePointer {
  void *rawValue;

public:
  OpaquePointer() = delete;
  explicit OpaquePointer(void *rawValue);
  void *get() const;

  void releaseUnderlyingResource();
};

} // namespace SwiftHDL

#endif // SWIFT_HDL_BINDINGS_SUPPORT_OPAQUE_POINTER
