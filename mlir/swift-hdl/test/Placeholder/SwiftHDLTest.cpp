// RUN: swift-hdl-test 2>&1 | FileCheck %s

#include <llvm/Support/ThreadPool.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/IR/BuiltinAttributes.h>

using namespace mlir;

int main() {
  // CHECK-LABEL: @placeholderTest
  llvm::errs() << "@placeholderTest\n";

  // CHECK-NEXT: Hello, MLIR!
  auto *threadPool = new llvm::ThreadPool();
  auto context = MLIRContext(MLIRContext::Threading::DISABLED);
  context.setThreadPool(*threadPool);

  auto attr = StringAttr::get(&context, "Hello, MLIR!");
  attr.dump();
  delete threadPool;
}
