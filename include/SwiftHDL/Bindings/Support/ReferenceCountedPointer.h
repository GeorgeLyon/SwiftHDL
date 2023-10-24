#ifndef SWIFT_HDL_BINDINGS_SUPPORT_REFERENCE_COUNTED_POINTER_H_
#define SWIFT_HDL_BINDINGS_SUPPORT_REFERENCE_COUNTED_POINTER_H_

namespace SwiftHDL {

class ReferenceCountedPointer {
public:
  // -- Lifecycle
  ~ReferenceCountedPointer();
  ReferenceCountedPointer(const ReferenceCountedPointer &other);
  ReferenceCountedPointer operator=(const ReferenceCountedPointer &other);

protected:
  class Implementation;
  explicit ReferenceCountedPointer(Implementation *impl);
  Implementation *get() const;

private:
  // Access to the implementation must go through `get()`
  mutable Implementation *impl;
};

} // namespace SwiftHDL

#endif // SWIFT_HDL_BINDINGS_SUPPORT_REFERENCE_COUNTED_POINTER_H_
