//===- Cosim.h - ESI C++ cosimulation backend -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is a specialization of the ESI C++ API (backend) for connection into a
// simulation of an ESI system. Currently uses Cap'nProto RPC, but that could
// change. Requires Cap'nProto C++ library.
//
// DO NOT EDIT!
// This file is distributed as part of an ESI package. The source for this file
// should always be modified within CIRCT (lib/dialect/ESI/runtime/cpp/esi.h).
//
//===----------------------------------------------------------------------===//

// NOLINTNEXTLINE(llvm-header-guard)
#ifndef ESI_BACKENDS_COSIM_H
#define ESI_BACKENDS_COSIM_H

#include "esi/Accelerator.h"

#include <memory>

namespace esi {
namespace backends {
namespace cosim {

/// Connect to an ESI simulation.
class CosimAccelerator : public esi::Accelerator {
public:
  CosimAccelerator(std::string hostname, uint16_t port);
  static std::unique_ptr<Accelerator> connect(std::string connectionString);

protected:
  virtual Service *createService(Service::Type service) override;

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace cosim
} // namespace backends
} // namespace esi

#endif // ESI_BACKENDS_COSIM_H
