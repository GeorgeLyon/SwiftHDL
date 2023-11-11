#pragma once

#include <swift-hdl/SwiftHDL.h>

// This should all probably all be moved into swift-hdl/SwiftHDL.h at some point
#include <circt/Dialect/Comb/CombDialect.h>
#include <circt/Dialect/FIRRTL/FIRRTLDialect.h>
#include <circt/Dialect/HW/HWDialect.h>
#include <circt/Dialect/SV/SVDialect.h>
#include <circt/Dialect/Seq/SeqDialect.h>
#include <llvm/Support/ThreadPool.h>

namespace CxxSwiftHDL {
// -- ThreadPool
inline llvm::ThreadPool *createThreadPool() { return new llvm::ThreadPool(); }
inline void destroyThreadPool(llvm::ThreadPool *threadPool) {
  delete threadPool;
}

// -- MLIRContext
inline mlir::MLIRContext *createMLIRContext(llvm::ThreadPool *threadPool) {
  auto *context = new mlir::MLIRContext(mlir::MLIRContext::Threading::DISABLED);
  context->setThreadPool(*threadPool);
  return context;
}
inline void destroyMLIRContext(mlir::MLIRContext *context) { delete context; }

// -- Dialects
inline void loadSwiftHDLDialects(mlir::MLIRContext *context) {
  context->loadDialect<circt::firrtl::FIRRTLDialect, circt::hw::HWDialect,
                       circt::comb::CombDialect, circt::seq::SeqDialect,
                       circt::sv::SVDialect>();
}

} // namespace CxxSwiftHDL
