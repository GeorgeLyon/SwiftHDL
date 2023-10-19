//===- IbisDialect.cpp - Implementation of Ibis dialect -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "circt/Dialect/Ibis/IbisDialect.h"
#include "circt/Dialect/Ibis/IbisOps.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace circt;
using namespace ibis;

// Pull in the dialect definition.
#include "circt/Dialect/Ibis/IbisDialect.cpp.inc"

void IbisDialect::initialize() {
  registerTypes();
  registerAttributes();

  // Register operations.
  addOperations<
#define GET_OP_LIST
#include "circt/Dialect/Ibis/Ibis.cpp.inc"
      >();
}

void IbisDialect::registerAttributes() {
  addAttributes<
#define GET_ATTRDEF_LIST
#include "circt/Dialect/Ibis/IbisAttributes.cpp.inc"
      >();
}

// Provide implementations for the enums we use.
#include "circt/Dialect/Ibis/IbisEnums.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "circt/Dialect/Ibis/IbisAttributes.cpp.inc"
