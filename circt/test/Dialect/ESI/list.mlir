// RUN: circt-opt %s -verify-diagnostics | circt-opt -verify-diagnostics | FileCheck %s

// CHECK: hw.module.extern @ListModule(in %listOfInts : !esi.list<i4>) 
hw.module.extern @ListModule(in %listOfInts: !esi.list<i4>) 
