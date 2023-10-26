#pragma once

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
