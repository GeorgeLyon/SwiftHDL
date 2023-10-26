#include "ScopedPassManager.h"

using namespace SwiftHDL;
using namespace mlir;

// -- Constructors

ScopedPassManager::ScopedPassManager(const ScopedBuilder &builder)
    : ReferenceCountedPointer(new Implementation(builder)) {}

// -- Accessors

PassManager *ScopedPassManager::operator->() { return &get()->passManager; }

ScopedBuilder ScopedPassManager::getBuilder() const { return get()->builder; }

void ScopedPassManager::setShouldInvalidateBuilderAfterRun(bool flag) {
  get()->shouldInvalidateBuilderAfterRun = flag;
}

LogicalResult ScopedPassManager::run(Operation *op) {
  auto impl = get();
  auto result = impl->passManager.run(op);
  if (impl->shouldInvalidateBuilderAfterRun) {
    impl->builder.invalidate();
  }
  return result;
}

// -- Conversion to/from OpaquePointer

ScopedPassManager ScopedPassManager::getFromOpaquePointer(OpaquePointer ptr) {
  return ScopedPassManager(Implementation::getFromOpaquePointer(ptr));
}

OpaquePointer ScopedPassManager::toRetainedOpaquePointer() {
  auto impl = get();
  impl->retain();
  return impl->toOpaquePointer();
}

// -- Reference Counted Pointer Implementation

ScopedPassManager::Implementation *ScopedPassManager::get() const {
  return static_cast<ScopedPassManager::Implementation *>(
      ReferenceCountedPointer::get());
}
