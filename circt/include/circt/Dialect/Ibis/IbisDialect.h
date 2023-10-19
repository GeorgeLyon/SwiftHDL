//===- IbisDialect.h - Definition of Ibis dialect ----------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef CIRCT_DIALECT_IBIS_IBISDIALECT_H
#define CIRCT_DIALECT_IBIS_IBISDIALECT_H

#include "mlir/IR/Dialect.h"

// Pull in the dialect definition.
#include "circt/Dialect/Ibis/IbisDialect.h.inc"

// Pull in all enum type definitions and utility function declarations.
#include "circt/Dialect/Ibis/IbisEnums.h.inc"

#define GET_ATTRDEF_CLASSES
#include "circt/Dialect/Ibis/IbisAttributes.h.inc"

#endif // CIRCT_DIALECT_IBIS_IBISDIALECT_H
