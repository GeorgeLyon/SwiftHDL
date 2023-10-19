//===- OMAttributes.h - Object Model attribute declarations ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Object Model attribute declarations.
//
//===----------------------------------------------------------------------===//

#ifndef CIRCT_DIALECT_OM_OMATTRIBUTES_H
#define CIRCT_DIALECT_OM_OMATTRIBUTES_H

#include "circt/Dialect/OM/OMDialect.h"
#include "mlir/IR/BuiltinAttributes.h"

namespace circt::om {

/// A module name, and the name of an instance inside that module.
struct PathElement {
  PathElement(mlir::StringAttr module, mlir::StringAttr instance)
      : module(module), instance(instance) {}

  bool operator==(const PathElement &rhs) const {
    return module == rhs.module && instance == rhs.instance;
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  friend llvm::hash_code hash_value(const PathElement &arg) {
    return ::llvm::hash_combine(arg.module, arg.instance);
  }

  mlir::StringAttr module;
  mlir::StringAttr instance;
};
} // namespace circt::om

#include "circt/Dialect/HW/HWAttributes.h"
#include "mlir/IR/Attributes.h"

#define GET_ATTRDEF_CLASSES
#include "circt/Dialect/OM/OMAttributes.h.inc"

#endif // CIRCT_DIALECT_OM_OMATTRIBUTES_H
