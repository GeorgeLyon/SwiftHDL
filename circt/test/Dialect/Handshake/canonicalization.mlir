// NOTE: Assertions have been autogenerated by utils/generate-test-checks.py

// RUN: circt-opt -split-input-file -canonicalize='top-down=true region-simplify=true' %s | FileCheck %s

// CHECK-LABEL:   handshake.func @simple(
// CHECK-SAME:                           %[[VAL_0:.*]]: none, ...) -> none
// CHECK:           %[[VAL_1:.*]] = constant %[[VAL_0]] {value = 1 : index} : index
// CHECK:           %[[VAL_2:.*]]:2 = fork [2] %[[VAL_0]] : none
// CHECK:           %[[VAL_3:.*]] = constant %[[VAL_2]]#0 {value = 42 : index} : index
// CHECK:           %[[VAL_4:.*]] = arith.addi %[[VAL_1]], %[[VAL_3]] : index
// CHECK:           sink %[[VAL_4]] : index
// CHECK:           return %[[VAL_2]]#1 : none
// CHECK:         }
handshake.func @simple(%arg0: none, ...) -> none {
  %0 = constant %arg0 {value = 1 : index} : index
  %1 = br %arg0 : none
  %2 = br %0 : index
  %3 = merge %1 : none
  %4 = merge %2 : index
  %5:2 = fork [2] %3 : none
  %6 = constant %5#0 {value = 42 : index} : index
  %7 = arith.addi %4, %6 : index
  sink %7 : index
  handshake.return %5#1 : none
}

// -----

// CHECK:   handshake.func @cmerge_with_control_used(%[[VAL_0:.*]]: none, %[[VAL_1:.*]]: none, %[[VAL_2:.*]]: none, ...) -> (none, index, none)
// CHECK:           %[[VAL_3:.*]], %[[VAL_4:.*]] = control_merge %[[VAL_0]], %[[VAL_1]] : none, index
// CHECK:           return %[[VAL_3]], %[[VAL_4]], %[[VAL_2]] : none, index, none
// CHECK:         }
handshake.func @cmerge_with_control_used(%arg0: none, %arg1: none, %arg2: none) -> (none, index, none) {
  %result, %index = control_merge %arg0, %arg1 : none, index
  handshake.return %result, %index, %arg2 : none, index, none
}


// -----

// CHECK:   handshake.func @cmerge_with_control_sunk(%[[VAL_0:.*]]: none, %[[VAL_1:.*]]: none, %[[VAL_2:.*]]: none, ...) -> (none, none)
// CHECK:           %[[VAL_3:.*]] = merge %[[VAL_0]], %[[VAL_1]] : none
// CHECK:           return %[[VAL_3]], %[[VAL_2]] : none, none
// CHECK:         }
handshake.func @cmerge_with_control_sunk(%arg0: none, %arg1: none, %arg2: none) -> (none, none) {
  %result, %index = control_merge %arg0, %arg1 : none, index
  sink %index : index
  handshake.return %result, %arg2 : none, none
}

// -----

// CHECK:   handshake.func @cmerge_with_control_ignored(%[[VAL_0:.*]]: none, %[[VAL_1:.*]]: none, %[[VAL_2:.*]]: none, ...) -> (none, none)
// CHECK:           %[[VAL_3:.*]] = merge %[[VAL_0]], %[[VAL_1]] : none
// CHECK:           return %[[VAL_3]], %[[VAL_2]] : none, none
// CHECK:         }
handshake.func @cmerge_with_control_ignored(%arg0: none, %arg1: none, %arg2: none) -> (none, none) {
  %result, %index = control_merge %arg0, %arg1 : none, index
  handshake.return %result, %arg2 : none, none
}

// -----

// CHECK-LABEL:   handshake.func @sunk_constant(
// CHECK-SAME:                                  %[[VAL_0:.*]]: none, ...) -> none
// CHECK:           return %[[VAL_0]] : none
// CHECK:         }
handshake.func @sunk_constant(%arg0: none) -> (none) {
  %0 = constant %arg0 { value = 24 : i8 } : i8
  sink %0 : i8
  handshake.return %arg0: none
}

// -----

// CHECK-LABEL:   handshake.func @unused_fork_result(
// CHECK-SAME:                                       %[[VAL_0:.*]]: none, ...) -> (none, none)
// CHECK:           %[[VAL_1:.*]]:2 = fork [2] %[[VAL_0]] : none
// CHECK:           return %[[VAL_1]]#0, %[[VAL_1]]#1 : none, none
// CHECK:         }
handshake.func @unused_fork_result(%arg0: none) -> (none, none) {
  %0:3 = fork [3] %arg0 : none
  handshake.return %0#0, %0#2 : none, none
}

// -----

// CHECK-LABEL:   handshake.func @simple_mux(
// CHECK-SAME:                               %[[VAL_0:.*]]: index,
// CHECK-SAME:                               %[[VAL_1:.*]]: none, ...) -> (none, none)
// CHECK:           return %[[VAL_1]], %[[VAL_1]] : none, none
// CHECK:         }
handshake.func @simple_mux(%c : index, %arg1: none) -> (none, none) {
  %0 = mux %c [%arg1, %arg1, %arg1] : index, none
  handshake.return %0, %arg1 : none, none
}


// -----

// CHECK-LABEL:   handshake.func @simple_mux2(
// CHECK-SAME:                                %[[VAL_0:.*]]: index,
// CHECK-SAME:                                %[[VAL_1:.*]]: none, ...) -> (none, none)
// CHECK:           %[[VAL_2:.*]] = buffer [2] seq %[[VAL_1]] : none
// CHECK:           return %[[VAL_2]], %[[VAL_1]] : none, none
// CHECK:         }
handshake.func @simple_mux2(%c : index, %arg1: none) -> (none, none) {
  %0:3 = fork [3] %arg1 : none
  %1 = buffer [2] seq %0#0 : none
  %2 = buffer [2] seq %0#1 : none
  %3 = mux %c [%1, %0#2] : index, none
  handshake.return %2, %3 : none, none
}

// -----

// CHECK-LABEL:   handshake.func @simple_mux3(
// CHECK-SAME:                                %[[VAL_0:.*]]: i32,
// CHECK-SAME:                                %[[VAL_1:.*]]: index,
// CHECK-SAME:                                %[[VAL_2:.*]]: none, ...) -> (i32, i32, none)
// CHECK:           %[[VAL_3:.*]]:2 = fork [2] %[[VAL_0]] : i32
// CHECK:           %[[VAL_4:.*]] = buffer [2] seq %[[VAL_3]]#1 : i32
// CHECK:           %[[VAL_5:.*]] = arith.addi %[[VAL_3]]#0, %[[VAL_4]] : i32
// CHECK:           return %[[VAL_5]], %[[VAL_0]], %[[VAL_2]] : i32, i32, none
// CHECK:         }
handshake.func @simple_mux3(%arg0 : i32, %c : index, %arg1: none) -> (i32, i32, none) {
  %0:3 = fork [3] %arg0 : i32
  %1:2 = fork [2] %0#0 : i32
  %3 = mux %c [%1#1, %0#1] : index, i32
  %4 = buffer [2] seq %1#1 : i32
  %5 = arith.addi %0#1, %4 : i32
  handshake.return %5, %3, %arg1 : i32, i32, none
}

// -----

// CHECK-LABEL:   handshake.func @fork_to_fork(
// CHECK-SAME:                                 %[[VAL_0:.*]]: i32,
// CHECK-SAME:                                 %[[VAL_1:.*]]: none, ...) -> (i32, i32, i32, none)
// CHECK:           %[[VAL_2:.*]]:3 = fork [3] %[[VAL_0]] : i32
// CHECK:           return %[[VAL_2]]#0, %[[VAL_2]]#1, %[[VAL_2]]#2, %[[VAL_1]] : i32, i32, i32, none
// CHECK:         }
handshake.func @fork_to_fork(%arg0 : i32, %arg1: none) -> (i32, i32, i32, none) {
  %0:2 = fork [2] %arg0 : i32
  %1:2 = fork [2] %0#0 : i32
  handshake.return %0#1, %1#0, %1#1, %arg1 : i32, i32, i32, none
}

// -----


// CHECK-LABEL:   handshake.func @sunk_buffer(
// CHECK-SAME:                                 %[[VAL_0:.*]]: i32,
// CHECK-SAME:                                 %[[VAL_1:.*]]: none, ...) -> none
// CHECK:           sink %[[VAL_0]] : i32
// CHECK:           return %[[VAL_1]] : none
// CHECK:         }
handshake.func @sunk_buffer(%arg0 : i32, %arg1: none) -> (none) {
  %0 = buffer [2] fifo %arg0  : i32
  sink %0 : i32
  return %arg1 : none
}

// -----

// CHECK-LABEL:   handshake.func @cbranch_into_mux_elim(
// CHECK-SAME:                                          %[[VAL_0:.*]]: i1,
// CHECK-SAME:                                          %[[VAL_1:.*]]: index,
// CHECK-SAME:                                          %[[VAL_2:.*]]: i32,
// CHECK-SAME:                                          %[[VAL_3:.*]]: none, ...) -> (i32, none)
// CHECK:           return %[[VAL_2]], %[[VAL_3]] : i32, none
// CHECK:         }
handshake.func @cbranch_into_mux_elim(%arg0 : i1, %arg1 : index, %arg2 : i32, %arg3: none) -> (i32, none) {
  %trueResult, %falseResult = cond_br %arg0, %arg2 : i32
  %0 = mux %arg1 [%trueResult, %falseResult] : index, i32
  handshake.return %0, %arg3 : i32, none
}

// -----

// CHECK-LABEL:   handshake.func @cbranch_into_mux_double(
// CHECK-SAME:                                            %[[VAL_0:.*]]: i1,
// CHECK-SAME:                                            %[[VAL_1:.*]]: index,
// CHECK-SAME:                                            %[[VAL_2:.*]]: i32,
// CHECK-SAME:                                            %[[VAL_3:.*]]: none, ...) -> (i32, i32, none)
// CHECK:           %[[VAL_4:.*]], %[[VAL_5:.*]] = cond_br %[[VAL_0]], %[[VAL_2]] : i32
// CHECK:           %[[VAL_6:.*]], %[[VAL_7:.*]] = cond_br %[[VAL_0]], %[[VAL_2]] : i32
// CHECK:           %[[VAL_8:.*]] = mux %[[VAL_1]] {{\[}}%[[VAL_4]], %[[VAL_7]]] : index, i32
// CHECK:           %[[VAL_9:.*]] = mux %[[VAL_1]] {{\[}}%[[VAL_6]], %[[VAL_5]]] : index, i32
// CHECK:           return %[[VAL_8]], %[[VAL_9]], %[[VAL_3]] : i32, i32, none
// CHECK:         }
handshake.func @cbranch_into_mux_double(%arg0 : i1, %arg1 : index, %arg2 : i32, %arg3: none) -> (i32, i32, none) {
  %trueResult0, %falseResult0 = cond_br %arg0, %arg2 : i32
  %trueResult1, %falseResult1 = cond_br %arg0, %arg2 : i32
  %0 = mux %arg1 [%trueResult0, %falseResult1] : index, i32
  %1 = mux %arg1 [%trueResult1, %falseResult0] : index, i32
  handshake.return %0, %1, %arg3 : i32, i32, none
}

// -----

// CHECK-LABEL:   handshake.func @cbranch_into_mux_extend(
// CHECK-SAME:                                            %[[VAL_0:.*]]: i1,
// CHECK-SAME:                                            %[[VAL_1:.*]]: index,
// CHECK-SAME:                                            %[[VAL_2:.*]]: i32,
// CHECK-SAME:                                            %[[VAL_3:.*]]: none, ...) -> (i32, none)
// CHECK:           %[[VAL_4:.*]], %[[VAL_5:.*]] = cond_br %[[VAL_0]], %[[VAL_2]] : i32
// CHECK:           %[[VAL_6:.*]] = mux %[[VAL_1]] {{\[}}%[[VAL_4]], %[[VAL_5]], %[[VAL_2]]] : index, i32
// CHECK:           return %[[VAL_6]], %[[VAL_3]] : i32, none
// CHECK:         }
handshake.func @cbranch_into_mux_extend(%arg0 : i1, %arg1 : index, %arg2 : i32, %arg4: none) -> (i32, none) {
  %trueResult, %falseResult = cond_br %arg0, %arg2 : i32
  %0 = mux %arg1 [%trueResult, %falseResult, %arg2] : index, i32
  handshake.return %0, %arg4 : i32, none
}

// -----

// CHECK-LABEL:   handshake.func @cbranch_into_mux_no_erase(
// CHECK-SAME:                                          %[[VAL_0:.*]]: i1,
// CHECK-SAME:                                          %[[VAL_1:.*]]: index,
// CHECK-SAME:                                          %[[VAL_2:.*]]: i32,
// CHECK-SAME:                                          %[[VAL_3:.*]]: none, ...) -> (i32, i32, none)
// CHECK:           %[[VAL_4:.*]], %[[VAL_5:.*]] = cond_br %[[VAL_0]], %[[VAL_2]] : i32
// CHECK:           %[[VAL_6:.*]] = arith.addi %[[VAL_4]], %[[VAL_4]] : i32
// CHECK:           return %[[VAL_2]], %[[VAL_6]], %[[VAL_3]] : i32, i32, none
// CHECK:         }
handshake.func @cbranch_into_mux_no_erase(%arg0 : i1, %arg1 : index, %arg2 : i32, %arg3: none) -> (i32, i32, none) {
  %trueResult, %falseResult = cond_br %arg0, %arg2 : i32
  %0 = mux %arg1 [%trueResult, %falseResult] : index, i32
  %1 = arith.addi %trueResult, %trueResult : i32 
  handshake.return %0, %1, %arg3 : i32, i32, none
}