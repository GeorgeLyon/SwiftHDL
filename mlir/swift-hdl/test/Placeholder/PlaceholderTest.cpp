// RUN: swift-hdl-test 2>&1 | FileCheck %s

#include <llvm/Support/raw_ostream.h>
#include <mlir/IR/BuiltinAttributes.h>

using namespace mlir;

int main() {
  // CHECK-LABEL: @placeholderTest
  llvm::errs() << "@placeholderTest\n";

  // CHECK-NEXT: Hello, MLIR!
  auto context = MLIRContext();
  auto attr = StringAttr::get(&context, "Hello, MLIR!");
  attr.dump();
  exit(0);
}
