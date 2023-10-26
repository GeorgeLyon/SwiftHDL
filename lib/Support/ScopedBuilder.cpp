#include "ScopedBuilder.h"

using namespace mlir;
using namespace SwiftHDL;

ScopedBuilder::Implementation *ScopedBuilder::get() const {
  return static_cast<ScopedBuilder::Implementation *>(
      ReferenceCountedPointer::get());
}

ScopedBuilder::ScopedBuilder(const ScopedContext &context)
    : ReferenceCountedPointer(new Implementation(context)) {}

void ScopedBuilder::invalidate() { get()->isValid = false; }

ScopedContext ScopedBuilder::getContext() const { return get()->context; }

ScopedBuilder ScopedBuilder::getFromOpaquePointer(OpaquePointer ptr) {
  return ScopedBuilder(Implementation::getFromOpaquePointer(ptr));
}

OpaquePointer ScopedBuilder::toRetainedOpaquePointer() {
  auto impl = get();
  impl->retain();
  return impl->toOpaquePointer();
}

OpBuilder *ScopedBuilder::operator->() { return &get()->builder; }

// -- Wrapping and Unwrapping

OpaquePointer ScopedBuilder::wrap(const Type &type) const {
  return getContext().wrap(type);
}
OpaquePointer ScopedBuilder::wrap(const Attribute &attribute) const {
  return getContext().wrap(attribute);
}
OpaquePointer ScopedBuilder::wrap(const Location &location) const {
  return getContext().wrap(location);
}
OpaquePointer ScopedBuilder::wrap(const OpState &opState) const {
  auto container = new Implementation::OpStateContainer(*this, opState);
  return container->toOpaquePointer();
}
OpaquePointer ScopedBuilder::wrap(Region *region) const {
  auto container = new Implementation::RegionContainer(*this, region);
  return container->toOpaquePointer();
}
OpaquePointer ScopedBuilder::wrap(Block *block) const {
  auto container = new Implementation::BlockContainer(*this, block);
  return container->toOpaquePointer();
}
OpaquePointer ScopedBuilder::wrap(const Value &value) const {
  auto container = new Implementation::ValueContainer(*this, value);
  return container->toOpaquePointer();
}

OpState ScopedBuilder::unwrapOpState(OpaquePointer opaquePointer) const {
  auto container =
      Implementation::OpStateContainer::getFromOpaquePointer(opaquePointer);

  assert(container->isValidInScope(*this));
  return container->opState;
}
Region *ScopedBuilder::unwrapRegion(OpaquePointer opaquePointer) const {
  auto container =
      Implementation::RegionContainer::getFromOpaquePointer(opaquePointer);
  assert(container->isValidInScope(*this));
  return container->region;
}
Block *ScopedBuilder::unwrapBlock(OpaquePointer opaquePointer) const {
  auto container =
      Implementation::BlockContainer::getFromOpaquePointer(opaquePointer);
  assert(container->isValidInScope(*this));
  return container->block;
}
Value ScopedBuilder::unwrapValue(OpaquePointer opaquePointer) const {
  auto container =
      Implementation::ValueContainer::getFromOpaquePointer(opaquePointer);
  assert(container->isValidInScope(*this));
  return container->value;
}
