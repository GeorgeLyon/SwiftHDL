
#include "OpaquePointer.h"
#include "ScopedBuilder.h"
#include "ScopedContext.h"
#include "ScopedPassManager.h"

using namespace SwiftHDL;

OpaquePointer::OpaquePointer(void *rawValue) : rawValue(rawValue) {
  assert(rawValue);
}

void *OpaquePointer::get() const {
  assert(rawValue);
  return rawValue;
}

void OpaquePointer::releaseUnderlyingResource() {
  assert(rawValue);
  auto any = static_cast<AnyOpaquePointerRepresentable *>(rawValue);
  using Kind = AnyOpaquePointerRepresentable::Kind;
  switch (any->_kind) {
  case Kind::Context: {
    ScopedContext::Implementation::getFromOpaquePointer(*this)->release();
    break;
  }
  case Kind::Builder: {
    ScopedBuilder::Implementation::getFromOpaquePointer(*this)->release();
    break;
  }
  case Kind::PassManager: {
    ScopedPassManager::Implementation::getFromOpaquePointer(*this)->release();
    break;
  }
  case Kind::Type: {
    delete ScopedContext::Implementation::TypeContainer::getFromOpaquePointer(
        *this);
    break;
  }
  case Kind::Attribute: {
    delete ScopedContext::Implementation::AttributeContainer::
        getFromOpaquePointer(*this);
    break;
  }
  case Kind::OpState: {
    delete ScopedBuilder::Implementation::OpStateContainer::
        getFromOpaquePointer(*this);
    break;
  }
  case Kind::Region: {
    delete ScopedBuilder::Implementation::RegionContainer::getFromOpaquePointer(
        *this);
    break;
  }
  case Kind::Block: {
    delete ScopedBuilder::Implementation::BlockContainer::getFromOpaquePointer(
        *this);
    break;
  }
  case Kind::Value: {
    delete ScopedBuilder::Implementation::ValueContainer::getFromOpaquePointer(
        *this);
    break;
  }
  }
  rawValue = nullptr;
}
