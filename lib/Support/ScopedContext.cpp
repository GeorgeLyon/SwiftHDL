#include "ScopedContext.h"

using namespace mlir;
using namespace SwiftHDL;

ScopedContext::Implementation *ScopedContext::get() const {
  return static_cast<ScopedContext::Implementation *>(
      ReferenceCountedPointer::get());
}

ScopedContext ScopedContext::create() {
  return ScopedContext(new Implementation());
}

ScopedContext::ScopedContext() : ScopedContext(new Implementation()) {}

ScopedContext ScopedContext::getFromOpaquePointer(OpaquePointer ptr) {
  return ScopedContext(Implementation::getFromOpaquePointer(ptr));
}

OpaquePointer ScopedContext::toRetainedOpaquePointer() {
  auto impl = get();
  impl->retain();
  return impl->toOpaquePointer();
}

ScopedContext::operator mlir::MLIRContext *() const {
  return &get()->mlirContext;
}

void *ScopedContext::getUserData() const { return get()->userData; }

void ScopedContext::setUserData(void *userData,
                                void (*userDataDeleter)(void *)) {
  auto impl = get();
  if (impl->userDataDeleter)
    impl->userDataDeleter(impl->userData);
  impl->userData = userData;
  impl->userDataDeleter = userDataDeleter;
}

OpaquePointer ScopedContext::wrap(const Attribute &attribute) const {
  auto container = new Implementation::AttributeContainer(*this, attribute);
  return container->toOpaquePointer();
}
OpaquePointer ScopedContext::wrap(const Location &location) const {
  return wrap(llvm::cast<Attribute>(location));
}
OpaquePointer ScopedContext::wrap(const Type &type) const {
  auto container = new Implementation::TypeContainer(*this, type);
  return container->toOpaquePointer();
}

Attribute ScopedContext::unwrapAttribute(OpaquePointer opaquePointer) const {
  auto container =
      Implementation::AttributeContainer::getFromOpaquePointer(opaquePointer);
  assert(container->isValidInContext(*this));
  return container->attribute;
}
Type ScopedContext::unwrapType(OpaquePointer opaquePointer) const {
  auto container =
      Implementation::TypeContainer::getFromOpaquePointer(opaquePointer);
  assert(container->isValidInContext(*this));
  return container->type;
}
