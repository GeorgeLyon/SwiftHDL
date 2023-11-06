// NOTE: Assertions have been autogenerated by utils/generate-test-checks.py
// RUN: mlir-opt %s -sparsification | FileCheck %s

#SV = #sparse_tensor.encoding<{ map = (d0) -> (d0 : compressed) }>

#trait2 = {
  indexing_maps = [
    affine_map<(i) -> (i)>,  // a
    affine_map<(i) -> (i)>,  // b
    affine_map<(i) -> (i)>   // x (out)
  ],
  iterator_types = ["parallel"],
  doc = "x(i) = a(i) OP b(i)"
}

#traitc = {
  indexing_maps = [
    affine_map<(i) -> (i)>,  // a
    affine_map<(i) -> (i)>   // x (out)
  ],
  iterator_types = ["parallel"],
  doc = "x(i) = a(i) OP c"
}

// CHECK-LABEL:   func @add(
// CHECK-SAME:              %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:              %[[VAL_1:.*]]: tensor<32xi64>,
// CHECK-SAME:              %[[VAL_2:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 32 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_5:.*]] = arith.constant true
// CHECK-DAG:           %[[VAL_6:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_8:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_9:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_10:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_11:.*]] = bufferization.to_memref %[[VAL_2]] : memref<32xi64>
// CHECK:           %[[VAL_12:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_6]]] : memref<?xindex>
// CHECK:           %[[VAL_14:.*]]:2 = scf.while (%[[VAL_15:.*]] = %[[VAL_12]], %[[VAL_16:.*]] = %[[VAL_4]]) : (index, index) -> (index, index) {
// CHECK:             %[[VAL_17:.*]] = arith.cmpi ult, %[[VAL_15]], %[[VAL_13]] : index
// CHECK:             scf.condition(%[[VAL_17]]) %[[VAL_15]], %[[VAL_16]] : index, index
// CHECK:           } do {
// CHECK:           ^bb0(%[[VAL_18:.*]]: index, %[[VAL_19:.*]]: index):
// CHECK:             %[[VAL_20:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_18]]] : memref<?xindex>
// CHECK:             %[[VAL_21:.*]] = arith.cmpi eq, %[[VAL_20]], %[[VAL_19]] : index
// CHECK:             scf.if %[[VAL_21]] {
// CHECK:               %[[VAL_22:.*]] = memref.load %[[VAL_9]]{{\[}}%[[VAL_18]]] : memref<?xi64>
// CHECK:               %[[VAL_23:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:               %[[VAL_24:.*]] = arith.addi %[[VAL_22]], %[[VAL_23]] : i64
// CHECK:               memref.store %[[VAL_24]], %[[VAL_11]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:             } else {
// CHECK:               scf.if %[[VAL_5]] {
// CHECK:                 %[[VAL_25:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:                 memref.store %[[VAL_25]], %[[VAL_11]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:               } else {
// CHECK:               }
// CHECK:             }
// CHECK:             %[[VAL_26:.*]] = arith.cmpi eq, %[[VAL_20]], %[[VAL_19]] : index
// CHECK:             %[[VAL_27:.*]] = arith.addi %[[VAL_18]], %[[VAL_6]] : index
// CHECK:             %[[VAL_28:.*]] = arith.select %[[VAL_26]], %[[VAL_27]], %[[VAL_18]] : index
// CHECK:             %[[VAL_29:.*]] = arith.addi %[[VAL_19]], %[[VAL_6]] : index
// CHECK:             scf.yield %[[VAL_28]], %[[VAL_29]] : index, index
// CHECK:           }
// CHECK:           scf.for %[[VAL_30:.*]] = %[[VAL_31:.*]]#1 to %[[VAL_3]] step %[[VAL_6]] {
// CHECK:             %[[VAL_32:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_30]]] : memref<32xi64>
// CHECK:             memref.store %[[VAL_32]], %[[VAL_11]]{{\[}}%[[VAL_30]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_33:.*]] = bufferization.to_tensor %[[VAL_11]] : memref<32xi64>
// CHECK:           return %[[VAL_33]] : tensor<32xi64>
// CHECK:         }
func.func @add(%arga: tensor<32xi64, #SV>,
               %argb: tensor<32xi64>,
               %argx: tensor<32xi64>) -> tensor<32xi64> {
  %0 = linalg.generic #trait2
     ins(%arga, %argb: tensor<32xi64, #SV>, tensor<32xi64>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %b: i64, %x: i64):
        %0 = arith.addi %a, %b : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @sub(
// CHECK-SAME:              %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:              %[[VAL_1:.*]]: tensor<32xi64>,
// CHECK-SAME:              %[[VAL_2:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 32 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_5:.*]] = arith.constant true
// CHECK-DAG:           %[[VAL_6:.*]] = arith.constant 1 : index
// CHECK-DAG:           %[[VAL_7:.*]] = arith.constant 0 : i64
// CHECK:           %[[VAL_8:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_9:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_10:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_11:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_12:.*]] = bufferization.to_memref %[[VAL_2]] : memref<32xi64>
// CHECK:           %[[VAL_13:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           %[[VAL_14:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_6]]] : memref<?xindex>
// CHECK:           %[[VAL_15:.*]]:2 = scf.while (%[[VAL_16:.*]] = %[[VAL_13]], %[[VAL_17:.*]] = %[[VAL_4]]) : (index, index) -> (index, index) {
// CHECK:             %[[VAL_18:.*]] = arith.cmpi ult, %[[VAL_16]], %[[VAL_14]] : index
// CHECK:             scf.condition(%[[VAL_18]]) %[[VAL_16]], %[[VAL_17]] : index, index
// CHECK:           } do {
// CHECK:           ^bb0(%[[VAL_19:.*]]: index, %[[VAL_20:.*]]: index):
// CHECK:             %[[VAL_21:.*]] = memref.load %[[VAL_9]]{{\[}}%[[VAL_19]]] : memref<?xindex>
// CHECK:             %[[VAL_22:.*]] = arith.cmpi eq, %[[VAL_21]], %[[VAL_20]] : index
// CHECK:             scf.if %[[VAL_22]] {
// CHECK:               %[[VAL_23:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<?xi64>
// CHECK:               %[[VAL_24:.*]] = memref.load %[[VAL_11]]{{\[}}%[[VAL_20]]] : memref<32xi64>
// CHECK:               %[[VAL_25:.*]] = arith.subi %[[VAL_23]], %[[VAL_24]] : i64
// CHECK:               memref.store %[[VAL_25]], %[[VAL_12]]{{\[}}%[[VAL_20]]] : memref<32xi64>
// CHECK:             } else {
// CHECK:               scf.if %[[VAL_5]] {
// CHECK:                 %[[VAL_26:.*]] = memref.load %[[VAL_11]]{{\[}}%[[VAL_20]]] : memref<32xi64>
// CHECK:                 %[[VAL_27:.*]] = arith.subi %[[VAL_7]], %[[VAL_26]] : i64
// CHECK:                 memref.store %[[VAL_27]], %[[VAL_12]]{{\[}}%[[VAL_20]]] : memref<32xi64>
// CHECK:               } else {
// CHECK:               }
// CHECK:             }
// CHECK:             %[[VAL_28:.*]] = arith.cmpi eq, %[[VAL_21]], %[[VAL_20]] : index
// CHECK:             %[[VAL_29:.*]] = arith.addi %[[VAL_19]], %[[VAL_6]] : index
// CHECK:             %[[VAL_30:.*]] = arith.select %[[VAL_28]], %[[VAL_29]], %[[VAL_19]] : index
// CHECK:             %[[VAL_31:.*]] = arith.addi %[[VAL_20]], %[[VAL_6]] : index
// CHECK:             scf.yield %[[VAL_30]], %[[VAL_31]] : index, index
// CHECK:           }
// CHECK:           scf.for %[[VAL_32:.*]] = %[[VAL_33:.*]]#1 to %[[VAL_3]] step %[[VAL_6]] {
// CHECK:             %[[VAL_34:.*]] = memref.load %[[VAL_11]]{{\[}}%[[VAL_32]]] : memref<32xi64>
// CHECK:             %[[VAL_35:.*]] = arith.subi %[[VAL_7]], %[[VAL_34]] : i64
// CHECK:             memref.store %[[VAL_35]], %[[VAL_12]]{{\[}}%[[VAL_32]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_36:.*]] = bufferization.to_tensor %[[VAL_12]] : memref<32xi64>
// CHECK:           return %[[VAL_36]] : tensor<32xi64>
// CHECK:         }
func.func @sub(%arga: tensor<32xi64, #SV>,
               %argb: tensor<32xi64>,
               %argx: tensor<32xi64>) -> tensor<32xi64> {
  %0 = linalg.generic #trait2
     ins(%arga, %argb: tensor<32xi64, #SV>, tensor<32xi64>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %b: i64, %x: i64):
        %0 = arith.subi %a, %b : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @mul(
// CHECK-SAME:              %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:              %[[VAL_1:.*]]: tensor<32xi64>,
// CHECK-SAME:              %[[VAL_2:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = bufferization.to_memref %[[VAL_2]] : memref<32xi64>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_11:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_12:.*]] = %[[VAL_10]] to %[[VAL_11]] step %[[VAL_4]] {
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_12]]] : memref<?xindex>
// CHECK:             %[[VAL_14:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_12]]] : memref<?xi64>
// CHECK:             %[[VAL_15:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_13]]] : memref<32xi64>
// CHECK:             %[[VAL_16:.*]] = arith.muli %[[VAL_14]], %[[VAL_15]] : i64
// CHECK:             memref.store %[[VAL_16]], %[[VAL_9]]{{\[}}%[[VAL_13]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_17:.*]] = bufferization.to_tensor %[[VAL_9]] : memref<32xi64>
// CHECK:           return %[[VAL_17]] : tensor<32xi64>
// CHECK:         }
func.func @mul(%arga: tensor<32xi64, #SV>,
               %argb: tensor<32xi64>,
               %argx: tensor<32xi64>) -> tensor<32xi64> {
  %0 = linalg.generic #trait2
     ins(%arga, %argb: tensor<32xi64, #SV>, tensor<32xi64>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %b: i64, %x: i64):
        %0 = arith.muli %a, %b : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @divsbyc(
// CHECK-SAME:                  %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:                  %[[VAL_1:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_2:.*]] = arith.constant 2 : i64
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_11:.*]] = %[[VAL_9]] to %[[VAL_10]] step %[[VAL_4]] {
// CHECK:             %[[VAL_12:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_11]]] : memref<?xindex>
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_11]]] : memref<?xi64>
// CHECK:             %[[VAL_14:.*]] = arith.divsi %[[VAL_13]], %[[VAL_2]] : i64
// CHECK:             memref.store %[[VAL_14]], %[[VAL_8]]{{\[}}%[[VAL_12]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_15:.*]] = bufferization.to_tensor %[[VAL_8]] : memref<32xi64>
// CHECK:           return %[[VAL_15]] : tensor<32xi64>
// CHECK:         }
func.func @divsbyc(%arga: tensor<32xi64, #SV>,
                   %argx: tensor<32xi64>) -> tensor<32xi64> {
  %c = arith.constant 2 : i64
  %0 = linalg.generic #traitc
     ins(%arga: tensor<32xi64, #SV>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %x: i64):
        %0 = arith.divsi %a, %c : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @divubyc(
// CHECK-SAME:                  %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:                  %[[VAL_1:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_2:.*]] = arith.constant 2 : i64
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{.*}}}>>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_11:.*]] = %[[VAL_9]] to %[[VAL_10]] step %[[VAL_4]] {
// CHECK:             %[[VAL_12:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_11]]] : memref<?xindex>
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_11]]] : memref<?xi64>
// CHECK:             %[[VAL_14:.*]] = arith.divui %[[VAL_13]], %[[VAL_2]] : i64
// CHECK:             memref.store %[[VAL_14]], %[[VAL_8]]{{\[}}%[[VAL_12]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_15:.*]] = bufferization.to_tensor %[[VAL_8]] : memref<32xi64>
// CHECK:           return %[[VAL_15]] : tensor<32xi64>
// CHECK:         }
func.func @divubyc(%arga: tensor<32xi64, #SV>,
                   %argx: tensor<32xi64>) -> tensor<32xi64> {
  %c = arith.constant 2 : i64
  %0 = linalg.generic #traitc
     ins(%arga: tensor<32xi64, #SV>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %x: i64):
        %0 = arith.divui %a, %c : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @and(
// CHECK-SAME:              %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:              %[[VAL_1:.*]]: tensor<32xi64>,
// CHECK-SAME:              %[[VAL_2:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xi64>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = bufferization.to_memref %[[VAL_2]] : memref<32xi64>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_11:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_12:.*]] = %[[VAL_10]] to %[[VAL_11]] step %[[VAL_4]] {
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_12]]] : memref<?xindex>
// CHECK:             %[[VAL_14:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_12]]] : memref<?xi64>
// CHECK:             %[[VAL_15:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_13]]] : memref<32xi64>
// CHECK:             %[[VAL_16:.*]] = arith.andi %[[VAL_14]], %[[VAL_15]] : i64
// CHECK:             memref.store %[[VAL_16]], %[[VAL_9]]{{\[}}%[[VAL_13]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_17:.*]] = bufferization.to_tensor %[[VAL_9]] : memref<32xi64>
// CHECK:           return %[[VAL_17]] : tensor<32xi64>
// CHECK:         }
func.func @and(%arga: tensor<32xi64, #SV>,
               %argb: tensor<32xi64>,
               %argx: tensor<32xi64>) -> tensor<32xi64> {
  %0 = linalg.generic #trait2
     ins(%arga, %argb: tensor<32xi64, #SV>, tensor<32xi64>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %b: i64, %x: i64):
        %0 = arith.andi %a, %b : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @or(
// CHECK-SAME:             %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:             %[[VAL_1:.*]]: tensor<32xi64>,
// CHECK-SAME:             %[[VAL_2:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 32 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_5:.*]] = arith.constant true
// CHECK-DAG:           %[[VAL_6:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_8:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_9:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xi64>
// CHECK:           %[[VAL_10:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_11:.*]] = bufferization.to_memref %[[VAL_2]] : memref<32xi64>
// CHECK:           %[[VAL_12:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_6]]] : memref<?xindex>
// CHECK:           %[[VAL_14:.*]]:2 = scf.while (%[[VAL_15:.*]] = %[[VAL_12]], %[[VAL_16:.*]] = %[[VAL_4]]) : (index, index) -> (index, index) {
// CHECK:             %[[VAL_17:.*]] = arith.cmpi ult, %[[VAL_15]], %[[VAL_13]] : index
// CHECK:             scf.condition(%[[VAL_17]]) %[[VAL_15]], %[[VAL_16]] : index, index
// CHECK:           } do {
// CHECK:           ^bb0(%[[VAL_18:.*]]: index, %[[VAL_19:.*]]: index):
// CHECK:             %[[VAL_20:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_18]]] : memref<?xindex>
// CHECK:             %[[VAL_21:.*]] = arith.cmpi eq, %[[VAL_20]], %[[VAL_19]] : index
// CHECK:             scf.if %[[VAL_21]] {
// CHECK:               %[[VAL_22:.*]] = memref.load %[[VAL_9]]{{\[}}%[[VAL_18]]] : memref<?xi64>
// CHECK:               %[[VAL_23:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:               %[[VAL_24:.*]] = arith.ori %[[VAL_22]], %[[VAL_23]] : i64
// CHECK:               memref.store %[[VAL_24]], %[[VAL_11]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:             } else {
// CHECK:               scf.if %[[VAL_5]] {
// CHECK:                 %[[VAL_25:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:                 memref.store %[[VAL_25]], %[[VAL_11]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:               } else {
// CHECK:               }
// CHECK:             }
// CHECK:             %[[VAL_26:.*]] = arith.cmpi eq, %[[VAL_20]], %[[VAL_19]] : index
// CHECK:             %[[VAL_27:.*]] = arith.addi %[[VAL_18]], %[[VAL_6]] : index
// CHECK:             %[[VAL_28:.*]] = arith.select %[[VAL_26]], %[[VAL_27]], %[[VAL_18]] : index
// CHECK:             %[[VAL_29:.*]] = arith.addi %[[VAL_19]], %[[VAL_6]] : index
// CHECK:             scf.yield %[[VAL_28]], %[[VAL_29]] : index, index
// CHECK:           }
// CHECK:           scf.for %[[VAL_30:.*]] = %[[VAL_31:.*]]#1 to %[[VAL_3]] step %[[VAL_6]] {
// CHECK:             %[[VAL_32:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_30]]] : memref<32xi64>
// CHECK:             memref.store %[[VAL_32]], %[[VAL_11]]{{\[}}%[[VAL_30]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_33:.*]] = bufferization.to_tensor %[[VAL_11]] : memref<32xi64>
// CHECK:           return %[[VAL_33]] : tensor<32xi64>
// CHECK:         }
func.func @or(%arga: tensor<32xi64, #SV>,
              %argb: tensor<32xi64>,
              %argx: tensor<32xi64>) -> tensor<32xi64> {
  %0 = linalg.generic #trait2
     ins(%arga, %argb: tensor<32xi64, #SV>, tensor<32xi64>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %b: i64, %x: i64):
        %0 = arith.ori %a, %b : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @xor(
// CHECK-SAME:             %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:             %[[VAL_1:.*]]: tensor<32xi64>,
// CHECK-SAME:             %[[VAL_2:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 32 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_5:.*]] = arith.constant true
// CHECK-DAG:           %[[VAL_6:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_8:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_9:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xi64>
// CHECK:           %[[VAL_10:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_11:.*]] = bufferization.to_memref %[[VAL_2]] : memref<32xi64>
// CHECK:           %[[VAL_12:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_6]]] : memref<?xindex>
// CHECK:           %[[VAL_14:.*]]:2 = scf.while (%[[VAL_15:.*]] = %[[VAL_12]], %[[VAL_16:.*]] = %[[VAL_4]]) : (index, index) -> (index, index) {
// CHECK:             %[[VAL_17:.*]] = arith.cmpi ult, %[[VAL_15]], %[[VAL_13]] : index
// CHECK:             scf.condition(%[[VAL_17]]) %[[VAL_15]], %[[VAL_16]] : index, index
// CHECK:           } do {
// CHECK:           ^bb0(%[[VAL_18:.*]]: index, %[[VAL_19:.*]]: index):
// CHECK:             %[[VAL_20:.*]] = memref.load %[[VAL_8]]{{\[}}%[[VAL_18]]] : memref<?xindex>
// CHECK:             %[[VAL_21:.*]] = arith.cmpi eq, %[[VAL_20]], %[[VAL_19]] : index
// CHECK:             scf.if %[[VAL_21]] {
// CHECK:               %[[VAL_22:.*]] = memref.load %[[VAL_9]]{{\[}}%[[VAL_18]]] : memref<?xi64>
// CHECK:               %[[VAL_23:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:               %[[VAL_24:.*]] = arith.xori %[[VAL_22]], %[[VAL_23]] : i64
// CHECK:               memref.store %[[VAL_24]], %[[VAL_11]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:             } else {
// CHECK:               scf.if %[[VAL_5]] {
// CHECK:                 %[[VAL_25:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:                 memref.store %[[VAL_25]], %[[VAL_11]]{{\[}}%[[VAL_19]]] : memref<32xi64>
// CHECK:               } else {
// CHECK:               }
// CHECK:             }
// CHECK:             %[[VAL_26:.*]] = arith.cmpi eq, %[[VAL_20]], %[[VAL_19]] : index
// CHECK:             %[[VAL_27:.*]] = arith.addi %[[VAL_18]], %[[VAL_6]] : index
// CHECK:             %[[VAL_28:.*]] = arith.select %[[VAL_26]], %[[VAL_27]], %[[VAL_18]] : index
// CHECK:             %[[VAL_29:.*]] = arith.addi %[[VAL_19]], %[[VAL_6]] : index
// CHECK:             scf.yield %[[VAL_28]], %[[VAL_29]] : index, index
// CHECK:           }
// CHECK:           scf.for %[[VAL_30:.*]] = %[[VAL_31:.*]]#1 to %[[VAL_3]] step %[[VAL_6]] {
// CHECK:             %[[VAL_32:.*]] = memref.load %[[VAL_10]]{{\[}}%[[VAL_30]]] : memref<32xi64>
// CHECK:             memref.store %[[VAL_32]], %[[VAL_11]]{{\[}}%[[VAL_30]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_33:.*]] = bufferization.to_tensor %[[VAL_11]] : memref<32xi64>
// CHECK:           return %[[VAL_33]] : tensor<32xi64>
// CHECK:         }
func.func @xor(%arga: tensor<32xi64, #SV>,
               %argb: tensor<32xi64>,
               %argx: tensor<32xi64>) -> tensor<32xi64> {
  %0 = linalg.generic #trait2
     ins(%arga, %argb: tensor<32xi64, #SV>, tensor<32xi64>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %b: i64, %x: i64):
        %0 = arith.xori %a, %b : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @ashrbyc(
// CHECK-SAME:                  %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:                  %[[VAL_1:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_2:.*]] = arith.constant 2 : i64
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xi64>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_11:.*]] = %[[VAL_9]] to %[[VAL_10]] step %[[VAL_4]] {
// CHECK:             %[[VAL_12:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_11]]] : memref<?xindex>
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_11]]] : memref<?xi64>
// CHECK:             %[[VAL_14:.*]] = arith.shrsi %[[VAL_13]], %[[VAL_2]] : i64
// CHECK:             memref.store %[[VAL_14]], %[[VAL_8]]{{\[}}%[[VAL_12]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_15:.*]] = bufferization.to_tensor %[[VAL_8]] : memref<32xi64>
// CHECK:           return %[[VAL_15]] : tensor<32xi64>
// CHECK:         }
func.func @ashrbyc(%arga: tensor<32xi64, #SV>,
                   %argx: tensor<32xi64>) -> tensor<32xi64> {
  %c = arith.constant 2 : i64
  %0 = linalg.generic #traitc
     ins(%arga: tensor<32xi64, #SV>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %x: i64):
        %0 = arith.shrsi %a, %c : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @lsrbyc(
// CHECK-SAME:                 %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:                 %[[VAL_1:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_2:.*]] = arith.constant 2 : i64
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xi64>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_11:.*]] = %[[VAL_9]] to %[[VAL_10]] step %[[VAL_4]] {
// CHECK:             %[[VAL_12:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_11]]] : memref<?xindex>
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_11]]] : memref<?xi64>
// CHECK:             %[[VAL_14:.*]] = arith.shrui %[[VAL_13]], %[[VAL_2]] : i64
// CHECK:             memref.store %[[VAL_14]], %[[VAL_8]]{{\[}}%[[VAL_12]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_15:.*]] = bufferization.to_tensor %[[VAL_8]] : memref<32xi64>
// CHECK:           return %[[VAL_15]] : tensor<32xi64>
// CHECK:         }
func.func @lsrbyc(%arga: tensor<32xi64, #SV>,
                  %argx: tensor<32xi64>) -> tensor<32xi64> {
  %c = arith.constant 2 : i64
  %0 = linalg.generic #traitc
     ins(%arga: tensor<32xi64, #SV>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %x: i64):
        %0 = arith.shrui %a, %c : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

// CHECK-LABEL:   func @lslbyc(
// CHECK-SAME:                 %[[VAL_0:.*]]: tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>>,
// CHECK-SAME:                 %[[VAL_1:.*]]: tensor<32xi64>) -> tensor<32xi64> {
// CHECK-DAG:           %[[VAL_2:.*]] = arith.constant 2 : i64
// CHECK-DAG:           %[[VAL_3:.*]] = arith.constant 0 : index
// CHECK-DAG:           %[[VAL_4:.*]] = arith.constant 1 : index
// CHECK:           %[[VAL_5:.*]] = sparse_tensor.positions %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_6:.*]] = sparse_tensor.coordinates %[[VAL_0]] {level = 0 : index} : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xindex>
// CHECK:           %[[VAL_7:.*]] = sparse_tensor.values %[[VAL_0]] : tensor<32xi64, #sparse_tensor.encoding<{{{.*}}}>> to memref<?xi64>
// CHECK:           %[[VAL_8:.*]] = bufferization.to_memref %[[VAL_1]] : memref<32xi64>
// CHECK:           %[[VAL_9:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_3]]] : memref<?xindex>
// CHECK:           %[[VAL_10:.*]] = memref.load %[[VAL_5]]{{\[}}%[[VAL_4]]] : memref<?xindex>
// CHECK:           scf.for %[[VAL_11:.*]] = %[[VAL_9]] to %[[VAL_10]] step %[[VAL_4]] {
// CHECK:             %[[VAL_12:.*]] = memref.load %[[VAL_6]]{{\[}}%[[VAL_11]]] : memref<?xindex>
// CHECK:             %[[VAL_13:.*]] = memref.load %[[VAL_7]]{{\[}}%[[VAL_11]]] : memref<?xi64>
// CHECK:             %[[VAL_14:.*]] = arith.shli %[[VAL_13]], %[[VAL_2]] : i64
// CHECK:             memref.store %[[VAL_14]], %[[VAL_8]]{{\[}}%[[VAL_12]]] : memref<32xi64>
// CHECK:           }
// CHECK:           %[[VAL_15:.*]] = bufferization.to_tensor %[[VAL_8]] : memref<32xi64>
// CHECK:           return %[[VAL_15]] : tensor<32xi64>
// CHECK:         }
func.func @lslbyc(%arga: tensor<32xi64, #SV>,
                  %argx: tensor<32xi64>) -> tensor<32xi64> {
  %c = arith.constant 2 : i64
  %0 = linalg.generic #traitc
     ins(%arga: tensor<32xi64, #SV>)
    outs(%argx: tensor<32xi64>) {
      ^bb(%a: i64, %x: i64):
        %0 = arith.shli %a, %c : i64
        linalg.yield %0 : i64
  } -> tensor<32xi64>
  return %0 : tensor<32xi64>
}

