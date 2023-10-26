// RUN: swift-hdl-bindings-support-test 2>&1 | FileCheck %s

#include "SwiftHDL/Support/OpaquePointer.h"
#include "SwiftHDL/Support/ScopedBuilder.h"
#include "SwiftHDL/Support/ScopedContext.h"
#include "SwiftHDL/Support/ScopedPassManager.h"

#include <llvm/Support/raw_ostream.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/Pass/PassManager.h>

using namespace mlir;
using namespace SwiftHDL;

void testContext();
void testBuilder();
void testPassManager();

int main() {
  testContext();
  testBuilder();
  testPassManager();
  return 0;
}

void testContext() {
  // CHECK-LABEL: @testContext
  llvm::errs() << "@testContext\n";

  std::optional<OpaquePointer> contextPointer = std::nullopt;
  std::optional<OpaquePointer> locationPointer = std::nullopt;

  {
    ScopedContext context = ScopedContext::create();
    contextPointer = context.toRetainedOpaquePointer();

    // CHECK-NEXT: Context Test
    StringAttr::get(context, "Context Test").dump();

    locationPointer = context.wrap(UnknownLoc::get(context));

    uint64_t userData = 42;
    context.setUserData(reinterpret_cast<void *>(userData), [](void *value) {
      assert(reinterpret_cast<uint64_t>(value) == 42);
      llvm::errs() << "User Data Deleted\n";
    });
  }

  {
    OpaquePointer pointer = contextPointer.value();
    ScopedContext context = ScopedContext::getFromOpaquePointer(pointer);
    pointer.releaseUnderlyingResource();

    // CHECK-NEXT: Context Opaque Pointer Test
    StringAttr::get(context, "Context Opaque Pointer Test").dump();

    // CHECK-NEXT: loc(unknown)
    context.unwrap<Location>(locationPointer.value())->dump();

    // CHECK-NEXT: 42
    llvm::errs() << reinterpret_cast<uint64_t>(context.getUserData()) << "\n";
  }

  // CHECK-NEXT: User Data Deleted
}

void testBuilder() {
  // CHECK-LABEL: @testBuilder
  llvm::errs() << "@testBuilder\n";

  std::optional<OpaquePointer> builderPointer = std::nullopt;
  std::optional<OpaquePointer> opPointer = std::nullopt;

  ScopedContext context = ScopedContext::create();
  {
    auto builder = ScopedBuilder(context);
    builderPointer = builder.toRetainedOpaquePointer();

    // CHECK-NEXT: Builder Test
    builder->getStringAttr("Builder Test").dump();

    opPointer =
        builder.wrap(builder->create<mlir::ModuleOp>(builder->getUnknownLoc()));
  }
  {
    OpaquePointer pointer = builderPointer.value();
    auto builder = ScopedBuilder::getFromOpaquePointer(pointer);
    pointer.releaseUnderlyingResource();

    // CHECK-NEXT: Builder Opaque Pointer Test
    builder->getStringAttr("Builder Opaque Pointer Test").dump();

    // CHECK-NEXT: module {
    // CHECK-NEXT: }
    builder.unwrap<Operation *>(opPointer.value())->dump();
  }
}

void testPassManager() {
  // CHECK-LABEL: @testPassManager
  llvm::errs() << "@testPassManager\n";

  std::optional<OpaquePointer> passManagerPointer = std::nullopt;

  ScopedContext context = ScopedContext::create();
  ScopedBuilder builder = ScopedBuilder(context);
  {
    auto passManager = ScopedPassManager(builder);
    passManagerPointer = passManager.toRetainedOpaquePointer();
  }
  {
    auto passManager =
        ScopedPassManager::getFromOpaquePointer(passManagerPointer.value());
    passManagerPointer->releaseUnderlyingResource();
    passManager->enableVerifier();
  }
}
