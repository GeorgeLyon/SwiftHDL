#include "ReferenceCountedPointer.h"

using namespace SwiftHDL;

ReferenceCountedPointer::ReferenceCountedPointer(Implementation *impl)
    : impl(impl) {
  impl->retain();
}

ReferenceCountedPointer::~ReferenceCountedPointer() { impl->release(); }

ReferenceCountedPointer::ReferenceCountedPointer(
    const ReferenceCountedPointer &other)
    : impl(other.impl) {
  impl->retain();
}

ReferenceCountedPointer
ReferenceCountedPointer::operator=(const ReferenceCountedPointer &other) {
  other.impl->retain();
  impl->release();
  impl = other.impl;
  return *this;
}

ReferenceCountedPointer::Implementation *ReferenceCountedPointer::get() const {
  return impl;
}
