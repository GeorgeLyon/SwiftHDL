#pragma once

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
