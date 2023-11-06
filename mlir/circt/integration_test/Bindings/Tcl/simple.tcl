# REQUIRES: bindings_tcl
# RUN: tclsh %s -- %TCL_PATH% %CIRCT_SOURCE% | FileCheck %s
load [lindex $argv 1]libcirct-tcl[info sharedlibextension]

set circuit [circt load MLIR [lindex $argv 2]/integration_test/Bindings/Tcl/Inputs/simple.mlir]
puts $circuit

# CHECK: module  {
# CHECK:   hw.module.extern @ichi(in %a : i2, in %b : i3, out c : i4, out d : i5)
# CHECK:   hw.module @owo(out owo_result : i32) {
# CHECK:     %c3_i32 = hw.constant 3 : i32
# CHECK:     hw.output %c3_i32 : i32
# CHECK:   }
# CHECK:   hw.module @uwu() {
# CHECK:     hw.output
# CHECK:   }
# CHECK:   hw.module @nya(in %nya_input : i32) {
# CHECK:     hw.instance "uwu1" @uwu() -> ()
# CHECK:     hw.output
# CHECK:   }
# CHECK:   hw.module @test(out test_result : i32) {
# CHECK:     %myArray1 = sv.wire  : !hw.inout<array<42xi8>>
# CHECK:     %owo1.owo_result = hw.instance "owo1" @owo() -> (owo_result: i32)
# CHECK:     hw.instance "nya1" @nya(nya_input: %owo1.owo_result: i32) -> ()
# CHECK:     hw.output %owo1.owo_result : i32
# CHECK:   }
# CHECK:   hw.module @always() {
# CHECK:     %true = hw.constant true
# CHECK:     %0 = sv.reg  : !hw.inout<i1>
# CHECK:     %false = hw.constant false
# CHECK:     sv.alwaysff(posedge %true)  {
# CHECK:       sv.passign %0, %false : i1
# CHECK:     }
# CHECK:     hw.output
# CHECK:   }
# CHECK: }
