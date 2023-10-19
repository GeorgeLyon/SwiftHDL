//===- SubsetInsertionOpInterfaceImpl.h - Tensor subsets ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_DIALECT_TENSOR_SUBSETINSERTIONOPINTERFACEIMPL_H
#define MLIR_DIALECT_TENSOR_SUBSETINSERTIONOPINTERFACEIMPL_H

namespace mlir {
class DialectRegistry;

namespace tensor {
void registerSubsetInsertionOpInterfaceExternalModels(
    DialectRegistry &registry);
} // namespace tensor
} // namespace mlir

#endif // MLIR_DIALECT_TENSOR_SUBSETINSERTIONOPINTERFACEIMPL_H
