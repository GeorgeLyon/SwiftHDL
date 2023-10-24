//===- PrintInstanceGraph.cpp - Print the instance graph --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//
//
// Print the module hierarchy.
//
//===----------------------------------------------------------------------===//

#include "PassDetails.h"
#include "circt/Dialect/FIRRTL/FIRRTLInstanceGraph.h"
#include "circt/Dialect/FIRRTL/Passes.h"
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace circt;
using namespace firrtl;

namespace {
struct PrintInstanceGraphPass
    : public PrintInstanceGraphBase<PrintInstanceGraphPass> {
  PrintInstanceGraphPass(raw_ostream &os) : os(os) {}
  void runOnOperation() override {
    auto circuitOp = getOperation();
    auto &instanceGraph = getAnalysis<InstanceGraph>();
    llvm::WriteGraph(os, &instanceGraph, /*ShortNames=*/false,
                     circuitOp.getName());
    markAllAnalysesPreserved();
  }
  raw_ostream &os;
};
} // end anonymous namespace

std::unique_ptr<mlir::Pass> circt::firrtl::createPrintInstanceGraphPass() {
  return std::make_unique<PrintInstanceGraphPass>(llvm::errs());
}
