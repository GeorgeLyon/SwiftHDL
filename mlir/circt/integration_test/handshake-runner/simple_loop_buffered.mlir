// RUN: circt-opt -lower-cf-to-handshake -handshake-materialize-forks-sinks %s \
// RUN: | circt-opt --handshake-insert-buffers="strategy=all" \
// RUN: | handshake-runner | FileCheck %s
// CHECK: 42
module {
  func.func @main() -> index {
    %c1 = arith.constant 1 : index
    %c42 = arith.constant 42 : index
    %c1_0 = arith.constant 1 : index
    cf.br ^bb1(%c1 : index)
  ^bb1(%0: index):	// 2 preds: ^bb0, ^bb2
    %1 = arith.cmpi slt, %0, %c42 : index
    cf.cond_br %1, ^bb2, ^bb3
  ^bb2:	// pred: ^bb1
    %2 = arith.addi %0, %c1_0 : index
    cf.br ^bb1(%2 : index)
  ^bb3:	// pred: ^bb1
    return %0 : index
  }
}
