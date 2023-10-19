// RUN: mlir-opt -allow-unregistered-dialect %s -split-input-file -verify-diagnostics | FileCheck %s

// CHECK: succeededSameOperandsElementType
func.func @succeededSameOperandsElementType(%t10x10 : tensor<10x10xf32>, %t1f: tensor<1xf32>, %v1: vector<1xf32>, %t1i: tensor<1xi32>, %sf: f32) {
  "test.same_operand_element_type"(%t1f, %t1f) : (tensor<1xf32>, tensor<1xf32>) -> tensor<1xi32>
  "test.same_operand_element_type"(%t1f, %t10x10) : (tensor<1xf32>, tensor<10x10xf32>) -> tensor<1xi32>
  "test.same_operand_element_type"(%t10x10, %v1) : (tensor<10x10xf32>, vector<1xf32>) -> tensor<1xi32>
  "test.same_operand_element_type"(%v1, %t1f) : (vector<1xf32>, tensor<1xf32>) -> tensor<1xi32>
  "test.same_operand_element_type"(%v1, %t1f) : (vector<1xf32>, tensor<1xf32>) -> tensor<121xi32>
  "test.same_operand_element_type"(%sf, %sf) : (f32, f32) -> i32
  "test.same_operand_element_type"(%sf, %t1f) : (f32, tensor<1xf32>) -> tensor<121xi32>
  "test.same_operand_element_type"(%sf, %v1) : (f32, vector<1xf32>) -> tensor<121xi32>
  "test.same_operand_element_type"(%sf, %t10x10) : (f32, tensor<10x10xf32>) -> tensor<121xi32>
  return
}

// -----

func.func @failedSameOperandElementType(%t1f: tensor<1xf32>, %t1i: tensor<1xi32>) {
  // expected-error@+1 {{requires the same element type for all operands}}
  "test.same_operand_element_type"(%t1f, %t1i) : (tensor<1xf32>, tensor<1xi32>) -> tensor<1xf32>
}

// -----

func.func @failedSameOperandAndResultElementType_no_operands() {
  // expected-error@+1 {{expected 2 operands, but found 0}}
  "test.same_operand_element_type"() : () -> tensor<1xf32>
}

// -----

func.func @failedSameOperandElementType_scalar_type_mismatch(%si: i32, %sf: f32) {
  // expected-error@+1 {{requires the same element type for all operands}}
  "test.same_operand_element_type"(%sf, %si) : (f32, i32) -> tensor<1xf32>
}

// -----

// CHECK: succeededSameOperandAndResultElementType
func.func @succeededSameOperandAndResultElementType(%t10x10 : tensor<10x10xf32>, %t1f: tensor<1xf32>, %v1: vector<1xf32>, %t1i: tensor<1xi32>, %sf: f32) {
  "test.same_operand_and_result_element_type"(%t1f, %t1f) : (tensor<1xf32>, tensor<1xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_element_type"(%t1f, %t10x10) : (tensor<1xf32>, tensor<10x10xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_element_type"(%t10x10, %v1) : (tensor<10x10xf32>, vector<1xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_element_type"(%v1, %t1f) : (vector<1xf32>, tensor<1xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_element_type"(%v1, %t1f) : (vector<1xf32>, tensor<1xf32>) -> tensor<121xf32>
  "test.same_operand_and_result_element_type"(%sf, %sf) : (f32, f32) -> f32
  "test.same_operand_and_result_element_type"(%sf, %t1f) : (f32, tensor<1xf32>) -> tensor<121xf32>
  "test.same_operand_and_result_element_type"(%sf, %v1) : (f32, vector<1xf32>) -> tensor<121xf32>
  "test.same_operand_and_result_element_type"(%sf, %t10x10) : (f32, tensor<10x10xf32>) -> tensor<121xf32>
  return
}

// -----

func.func @failedSameOperandAndResultElementType_operand_result_mismatch(%t1f: tensor<1xf32>) {
  // expected-error@+1 {{requires the same element type for all operands and results}}
  "test.same_operand_and_result_element_type"(%t1f, %t1f) : (tensor<1xf32>, tensor<1xf32>) -> tensor<1xi32>
}

// -----

func.func @failedSameOperandAndResultElementType_operand_mismatch(%t1f: tensor<1xf32>, %t1i: tensor<1xi32>) {
  // expected-error@+1 {{requires the same element type for all operands and results}}
  "test.same_operand_and_result_element_type"(%t1f, %t1i) : (tensor<1xf32>, tensor<1xi32>) -> tensor<1xf32>
}

// -----

func.func @failedSameOperandAndResultElementType_result_mismatch(%t1f: tensor<1xf32>) {
  // expected-error@+1 {{requires the same element type for all operands and results}}
  %0:2 = "test.same_operand_and_result_element_type"(%t1f) : (tensor<1xf32>) -> (tensor<1xf32>, tensor<1xi32>)
}

// -----

func.func @failedSameOperandAndResultElementType_no_operands() {
  // expected-error@+1 {{expected 1 or more operands}}
  "test.same_operand_and_result_element_type"() : () -> tensor<1xf32>
}

// -----

func.func @failedSameOperandAndResultElementType_no_results(%t1f: tensor<1xf32>) {
  // expected-error@+1 {{expected 1 or more results}}
  "test.same_operand_and_result_element_type"(%t1f) : (tensor<1xf32>) -> ()
}

// -----

// CHECK: succeededSameOperandShape
func.func @succeededSameOperandShape(%t10x10 : tensor<10x10xf32>, %t1: tensor<1xf32>, %m10x10 : memref<10x10xi32>, %tr: tensor<*xf32>) {
  "test.same_operand_shape"(%t1, %t1) : (tensor<1xf32>, tensor<1xf32>) -> ()
  "test.same_operand_shape"(%t10x10, %t10x10) : (tensor<10x10xf32>, tensor<10x10xf32>) -> ()
  "test.same_operand_shape"(%t1, %tr) : (tensor<1xf32>, tensor<*xf32>) -> ()
  "test.same_operand_shape"(%t10x10, %m10x10) : (tensor<10x10xf32>, memref<10x10xi32>) -> ()
  return
}

// -----

func.func @failedSameOperandShape_operand_mismatch(%t10x10 : tensor<10x10xf32>, %t1: tensor<1xf32>) {
  // expected-error@+1 {{requires the same shape for all operands}}
  "test.same_operand_shape"(%t1, %t10x10) : (tensor<1xf32>, tensor<10x10xf32>) -> ()
}

// -----

func.func @failedSameOperandShape_no_operands() {
  // expected-error@+1 {{expected 1 or more operands}}
  "test.same_operand_shape"() : () -> ()
}

// -----

// CHECK: succeededSameOperandAndResultShape
func.func @succeededSameOperandAndResultShape(%t10x10 : tensor<10x10xf32>, %t1: tensor<1xf32>, %tr: tensor<*xf32>, %t1d: tensor<?xf32>) {
  "test.same_operand_and_result_shape"(%t1, %t1) : (tensor<1xf32>, tensor<1xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_shape"(%t10x10, %t10x10) : (tensor<10x10xf32>, tensor<10x10xf32>) -> tensor<10x10xf32>
  "test.same_operand_and_result_shape"(%t1, %tr) : (tensor<1xf32>, tensor<*xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_shape"(%t1, %t1d) : (tensor<1xf32>, tensor<?xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_shape"(%t1, %t1d) : (tensor<1xf32>, tensor<?xf32>) -> memref<1xf32>

  return
}

// -----

func.func @failedSameOperandAndResultShape_operand_result_mismatch(%t10x10 : tensor<10x10xf32>, %t1: tensor<1xf32>) {
  // expected-error@+1 {{requires the same shape for all operands and results}}
  "test.same_operand_and_result_shape"(%t1, %t10x10) : (tensor<1xf32>, tensor<10x10xf32>) -> tensor<10x10xf32>
}

// -----

func.func @failedSameOperandAndResultShape_operand_result_mismatch(%t10 : tensor<10xf32>, %t1: tensor<?xf32>) {
  // expected-error@+1 {{requires the same shape for all operands and results}}
  "test.same_operand_and_result_shape"(%t1, %t10) : (tensor<?xf32>, tensor<10xf32>) -> tensor<3xf32>
}

// -----

func.func @failedSameOperandAndResultShape_no_operands() {
  // expected-error@+1 {{expected 1 or more operands}}
  "test.same_operand_and_result_shape"() : () -> (tensor<1xf32>)
}

// -----

func.func @failedSameOperandAndResultShape_no_operands(%t1: tensor<1xf32>) {
  // expected-error@+1 {{expected 1 or more results}}
  "test.same_operand_and_result_shape"(%t1) : (tensor<1xf32>) -> ()
}

// -----

// CHECK: succeededSameOperandAndResultType
func.func @succeededSameOperandAndResultType(%t10x10 : tensor<10x10xf32>, %t1: tensor<1xf32>, %tr: tensor<*xf32>, %t1d: tensor<?xf32>, %i32 : i32) {
  "test.same_operand_and_result_type"(%t1, %t1) : (tensor<1xf32>, tensor<1xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_type"(%t10x10, %t10x10) : (tensor<10x10xf32>, tensor<10x10xf32>) -> tensor<10x10xf32>
  "test.same_operand_and_result_type"(%t1, %tr) : (tensor<1xf32>, tensor<*xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_type"(%t1, %t1d) : (tensor<1xf32>, tensor<?xf32>) -> tensor<1xf32>
  "test.same_operand_and_result_type"(%i32, %i32) : (i32, i32) -> i32
  return
}

// -----

func.func @failedSameOperandAndResultType_operand_result_mismatch(%t10 : tensor<10xf32>, %t20 : tensor<20xf32>) {
  // expected-error@+1 {{requires the same type for all operands and results}}
  "test.same_operand_and_result_type"(%t10, %t20) : (tensor<10xf32>, tensor<20xf32>) -> tensor<10xf32>
  return
}

// -----

func.func @failedSameOperandAndResultType_encoding_mismatch(%t10 : tensor<10xf32>, %t20 : tensor<10xf32>) {
  // expected-error@+1 {{requires the same encoding for all operands and results}}
  "test.same_operand_and_result_type"(%t10, %t20) : (tensor<10xf32>, tensor<10xf32>) -> tensor<10xf32, "enc">
  return
}

// -----

func.func @failedElementwiseMappable_different_rankedness(%arg0: tensor<?xf32>, %arg1: tensor<*xf32>) {
  // expected-error@+1 {{all non-scalar operands/results must have the same shape and base type}}
  %0 = "test.elementwise_mappable"(%arg0, %arg1) : (tensor<?xf32>, tensor<*xf32>) -> tensor<*xf32>
  return
}

// -----

func.func @failedElementwiseMappable_different_rank(%arg0: tensor<?xf32>, %arg1: tensor<?x?xf32>) {
  // expected-error@+1 {{all non-scalar operands/results must have the same shape and base type}}
  %0 = "test.elementwise_mappable"(%arg0, %arg1) : (tensor<?xf32>, tensor<?x?xf32>) -> tensor<?x?xf32>
  return
}

// -----

func.func @elementwiseMappable_dynamic_shapes(%arg0: tensor<?xf32>,
    %arg1: tensor<5xf32>) {
  %0 = "test.elementwise_mappable"(%arg0, %arg1) :
      (tensor<?xf32>, tensor<5xf32>) -> tensor<?xf32>
  return
}

// -----

func.func @failedElementwiseMappable_different_base_type(%arg0: vector<2xf32>, %arg1: tensor<2xf32>) {
  // expected-error@+1 {{all non-scalar operands/results must have the same shape and base type}}
  %0 = "test.elementwise_mappable"(%arg0, %arg1) : (vector<2xf32>, tensor<2xf32>) -> tensor<2xf32>
  return
}

// -----

func.func @failedElementwiseMappable_non_scalar_output(%arg0: vector<2xf32>) {
  // expected-error@+1 {{if an operand is non-scalar, then there must be at least one non-scalar result}}
  %0 = "test.elementwise_mappable"(%arg0) : (vector<2xf32>) -> f32
  return
}

// -----

func.func @failedElementwiseMappable_non_scalar_result_all_scalar_input(%arg0: f32) {
  // expected-error@+1 {{if a result is non-scalar, then at least one operand must be non-scalar}}
  %0 = "test.elementwise_mappable"(%arg0) : (f32) -> tensor<f32>
  return
}

// -----

func.func @failedElementwiseMappable_mixed_scalar_non_scalar_results(%arg0: tensor<10xf32>) {
  // expected-error@+1 {{if an operand is non-scalar, then all results must be non-scalar}}
  %0, %1 = "test.elementwise_mappable"(%arg0) : (tensor<10xf32>) -> (f32, tensor<10xf32>)
  return
}

// -----

func.func @failedElementwiseMappable_zero_results(%arg0: tensor<10xf32>) {
  // expected-error@+1 {{if an operand is non-scalar, then there must be at least one non-scalar result}}
  "test.elementwise_mappable"(%arg0) : (tensor<10xf32>) -> ()
  return
}

// -----

func.func @failedElementwiseMappable_zero_operands() {
  // expected-error@+1 {{if a result is non-scalar, then at least one operand must be non-scalar}}
  "test.elementwise_mappable"() : () -> (tensor<6xf32>)
  return
}

// -----

func.func @succeededElementwiseMappable(%arg0: vector<2xf32>) {
  // Check that varying element types are allowed.
  // CHECK: test.elementwise_mappable
  %0 = "test.elementwise_mappable"(%arg0) : (vector<2xf32>) -> vector<2xf16>
  return
}

// -----

func.func @failedHasParent_wrong_parent() {
  "some.op"() ({
   // expected-error@+1 {{'test.child' op expects parent op 'test.parent'}}
    "test.child"() : () -> ()
  }) : () -> ()
}

// -----

// CHECK: succeededParentOneOf
func.func @succeededParentOneOf() {
  "test.parent"() ({
    "test.child_with_parent_one_of"() : () -> ()
    "test.finish"() : () -> ()
   }) : () -> ()
  return
}

// -----

// CHECK: succeededParent1OneOf
func.func @succeededParent1OneOf() {
  "test.parent1"() ({
    "test.child_with_parent_one_of"() : () -> ()
    "test.finish"() : () -> ()
   }) : () -> ()
  return
}

// -----

func.func @failedParentOneOf_wrong_parent1() {
  "some.otherop"() ({
    // expected-error@+1 {{'test.child_with_parent_one_of' op expects parent op to be one of 'test.parent, test.parent1'}}
    "test.child_with_parent_one_of"() : () -> ()
    "test.finish"() : () -> ()
   }) : () -> ()
}


// -----

func.func @failedSingleBlockImplicitTerminator_empty_block() {
   // expected-error@+1 {{'test.SingleBlockImplicitTerminator' op expects a non-empty block}}
  "test.SingleBlockImplicitTerminator"() ({
  ^entry:
  }) : () -> ()
}

// -----

func.func @failedSingleBlockImplicitTerminator_too_many_blocks() {
   // expected-error@+1 {{'test.SingleBlockImplicitTerminator' op expects region #0 to have 0 or 1 block}}
  "test.SingleBlockImplicitTerminator"() ({
  ^entry:
    "test.finish" () : () -> ()
  ^other:
    "test.finish" () : () -> ()
  }) : () -> ()
}

// -----

func.func @failedSingleBlockImplicitTerminator_missing_terminator() {
   // expected-error@+2 {{'test.SingleBlockImplicitTerminator' op expects regions to end with 'test.finish'}}
   // expected-note@+1 {{in custom textual format, the absence of terminator implies 'test.finish'}}
  "test.SingleBlockImplicitTerminator"() ({
  ^entry:
    "test.non_existent_op"() : () -> ()
  }) : () -> ()
}

// -----

// Test the invariants of operations with the Symbol Trait.

// expected-error@+1 {{op requires attribute 'sym_name'}}
"test.symbol"() {} : () -> ()

// -----

// expected-error@+1 {{invalid properties {sym_name = "foo_2", sym_visibility} for op test.symbol: Invalid attribute `sym_visibility` in property conversion: unit}}
"test.symbol"() <{sym_name = "foo_2", sym_visibility}> : () -> ()

// -----

// expected-error@+1 {{visibility expected to be one of ["public", "private", "nested"]}}
"test.symbol"() {sym_name = "foo_2", sym_visibility = "foo"} : () -> ()

// -----

"test.symbol"() {sym_name = "foo_3", sym_visibility = "nested"} : () -> ()
"test.symbol"() {sym_name = "foo_4", sym_visibility = "private"} : () -> ()
"test.symbol"() {sym_name = "foo_5", sym_visibility = "public"} : () -> ()
"test.symbol"() {sym_name = "foo_6"} : () -> ()

// -----

// Test that operation with the SymbolTable Trait define a new symbol scope.
"test.symbol_scope"() ({
  func.func private @foo()
  "test.finish" () : () -> ()
}) : () -> ()
func.func private @foo()

// -----

// Test that operation with the SymbolTable Trait fails with  too many blocks.
// expected-error@+1 {{op expects region #0 to have 0 or 1 blocks}}
"test.symbol_scope"() ({
  ^entry:
    "test.finish" () : () -> ()
  ^other:
    "test.finish" () : () -> ()
}) : () -> ()

// -----

func.func @failedMissingOperandSizeAttr(%arg: i32) {
  // expected-error @+1 {{op operand count (4) does not match with the total size (0) specified in attribute 'operandSegmentSizes'}}
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) : (i32, i32, i32, i32) -> ()
}

// -----

func.func @failedOperandSizeAttrWrongType(%arg: i32) {
  // expected-error @+1 {{op operand count (4) does not match with the total size (0) specified in attribute 'operandSegmentSizes'}}
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) {operandSegmentSizes = 10} : (i32, i32, i32, i32) -> ()
}

// -----

func.func @failedOperandSizeAttrWrongElementType(%arg: i32) {
  // expected-error @+1 {{op operand count (4) does not match with the total size (0) specified in attribute 'operandSegmentSizes'}}
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) {operandSegmentSizes = array<i64: 1, 1, 1, 1>} : (i32, i32, i32, i32) -> ()
}

// -----

func.func @failedOperandSizeAttrNegativeValue(%arg: i32) {
  // expected-error @+1 {{'operandSegmentSizes' attribute cannot have negative elements}}
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) {operandSegmentSizes = array<i32: 1, 1, -1, 1>} : (i32, i32, i32, i32) -> ()
}

// -----

func.func @failedOperandSizeAttrWrongTotalSize(%arg: i32) {
  // expected-error @+1 {{operand count (4) does not match with the total size (3) specified in attribute 'operandSegmentSizes'}}
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) {operandSegmentSizes = array<i32: 0, 1, 1, 1>} : (i32, i32, i32, i32) -> ()
}

// -----

func.func @failedOperandSizeAttrWrongCount(%arg: i32) {
  // expected-error @+1 {{test.attr_sized_operands' op operand count (4) does not match with the total size (0) specified in attribute 'operandSegmentSizes}}
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) {operandSegmentSizes = array<i32: 2, 1, 1>} : (i32, i32, i32, i32) -> ()
}

// -----

func.func @succeededOperandSizeAttr(%arg: i32) {
  // CHECK: test.attr_sized_operands
  "test.attr_sized_operands"(%arg, %arg, %arg, %arg) {operandSegmentSizes = array<i32: 0, 2, 1, 1>} : (i32, i32, i32, i32) -> ()
  return
}

// -----

func.func @failedMissingResultSizeAttr() {
  // expected-error @+1 {{op result count (4) does not match with the total size (0) specified in attribute 'resultSegmentSizes'}}
  %0:4 = "test.attr_sized_results"() : () -> (i32, i32, i32, i32)
}

// -----

func.func @failedResultSizeAttrWrongType() {
  // expected-error @+1 {{ op result count (4) does not match with the total size (0) specified in attribute 'resultSegmentSizes'}}
  %0:4 = "test.attr_sized_results"() {resultSegmentSizes = 10} : () -> (i32, i32, i32, i32)
}


// -----

func.func @failedResultSizeAttrWrongElementType() {
  // expected-error @+1 {{ op result count (4) does not match with the total size (0) specified in attribute 'resultSegmentSizes'}}
  %0:4 = "test.attr_sized_results"() {resultSegmentSizes = array<i64: 1, 1, 1, 1>} : () -> (i32, i32, i32, i32)
}

// -----

func.func @failedResultSizeAttrNegativeValue() {
  // expected-error @+1 {{'resultSegmentSizes' attribute cannot have negative elements}}
  %0:4 = "test.attr_sized_results"() {resultSegmentSizes = array<i32: 1, 1, -1, 1>} : () -> (i32, i32, i32, i32)
}

// -----

func.func @failedResultSizeAttrWrongTotalSize() {
  // expected-error @+1 {{result count (4) does not match with the total size (3) specified in attribute 'resultSegmentSizes'}}
  %0:4 = "test.attr_sized_results"() {resultSegmentSizes = array<i32: 0, 1, 1, 1>} : () -> (i32, i32, i32, i32)
}

// -----

func.func @failedResultSizeAttrWrongCount() {
  // expected-error @+1 {{ op result count (4) does not match with the total size (0) specified in attribute 'resultSegmentSizes'}}
  %0:4 = "test.attr_sized_results"() {resultSegmentSizes = array<i32: 2, 1, 1>} : () -> (i32, i32, i32, i32)
}

// -----

func.func @succeededResultSizeAttr() {
  // CHECK: test.attr_sized_results
  %0:4 = "test.attr_sized_results"() {resultSegmentSizes = array<i32: 0, 2, 1, 1>} : () -> (i32, i32, i32, i32)
  return
}

// -----

// CHECK-LABEL: @succeededOilistTrivial
func.func @succeededOilistTrivial() {
  // CHECK: test.oilist_with_keywords_only keyword
  test.oilist_with_keywords_only keyword
  // CHECK: test.oilist_with_keywords_only otherKeyword
  test.oilist_with_keywords_only otherKeyword
  // CHECK: test.oilist_with_keywords_only keyword otherKeyword
  test.oilist_with_keywords_only keyword otherKeyword
  // CHECK: test.oilist_with_keywords_only keyword otherKeyword
  test.oilist_with_keywords_only otherKeyword keyword
  // CHECK: test.oilist_with_keywords_only thirdKeyword
  test.oilist_with_keywords_only thirdKeyword
  // CHECK: test.oilist_with_keywords_only keyword thirdKeyword
  test.oilist_with_keywords_only keyword thirdKeyword
  return
}

// -----

// CHECK-LABEL: @succeededOilistSimple
func.func @succeededOilistSimple(%arg0 : i32, %arg1 : i32, %arg2 : i32) {
  // CHECK: test.oilist_with_simple_args keyword %{{.*}} : i32
  test.oilist_with_simple_args keyword %arg0 : i32
  // CHECK: test.oilist_with_simple_args otherKeyword %{{.*}} : i32
  test.oilist_with_simple_args otherKeyword %arg0 : i32
  // CHECK: test.oilist_with_simple_args thirdKeyword %{{.*}} : i32
  test.oilist_with_simple_args thirdKeyword %arg0 : i32

  // CHECK: test.oilist_with_simple_args keyword %{{.*}} : i32 otherKeyword %{{.*}} : i32
  test.oilist_with_simple_args keyword %arg0 : i32 otherKeyword %arg1 : i32
  // CHECK: test.oilist_with_simple_args keyword %{{.*}} : i32 thirdKeyword %{{.*}} : i32
  test.oilist_with_simple_args keyword %arg0 : i32 thirdKeyword %arg1 : i32
  // CHECK: test.oilist_with_simple_args otherKeyword %{{.*}} : i32 thirdKeyword %{{.*}} : i32
  test.oilist_with_simple_args thirdKeyword %arg0 : i32 otherKeyword %arg1 : i32

  // CHECK: test.oilist_with_simple_args keyword %{{.*}} : i32 otherKeyword %{{.*}} : i32 thirdKeyword %{{.*}} : i32
  test.oilist_with_simple_args keyword %arg0 : i32 otherKeyword %arg1 : i32 thirdKeyword %arg2 : i32
  // CHECK: test.oilist_with_simple_args keyword %{{.*}} : i32 otherKeyword %{{.*}} : i32 thirdKeyword %{{.*}} : i32
  test.oilist_with_simple_args otherKeyword %arg0 : i32 keyword %arg1 : i32 thirdKeyword %arg2 : i32
  // CHECK: test.oilist_with_simple_args keyword %{{.*}} : i32 otherKeyword %{{.*}} : i32 thirdKeyword %{{.*}} : i32
  test.oilist_with_simple_args otherKeyword %arg0 : i32 thirdKeyword %arg1 : i32 keyword %arg2 : i32
  return
}

// -----

// CHECK-LABEL: @succeededOilistVariadic
// CHECK-SAME: (%[[ARG0:.*]]: i32, %[[ARG1:.*]]: i32, %[[ARG2:.*]]: i32)
func.func @succeededOilistVariadic(%arg0: i32, %arg1: i32, %arg2: i32) {
  // CHECK: test.oilist_variadic_with_parens keyword(%[[ARG0]], %[[ARG1]] : i32, i32)
  test.oilist_variadic_with_parens keyword (%arg0, %arg1 : i32, i32)
  // CHECK: test.oilist_variadic_with_parens keyword(%[[ARG0]], %[[ARG1]] : i32, i32) otherKeyword(%[[ARG2]], %[[ARG1]] : i32, i32)
  test.oilist_variadic_with_parens otherKeyword (%arg2, %arg1 : i32, i32) keyword (%arg0, %arg1 : i32, i32)
  // CHECK: test.oilist_variadic_with_parens keyword(%[[ARG0]], %[[ARG1]] : i32, i32) otherKeyword(%[[ARG0]], %[[ARG1]] : i32, i32) thirdKeyword(%[[ARG2]], %[[ARG0]], %[[ARG1]] : i32, i32, i32)
  test.oilist_variadic_with_parens thirdKeyword (%arg2, %arg0, %arg1 : i32, i32, i32) keyword (%arg0, %arg1 : i32, i32) otherKeyword (%arg0, %arg1 : i32, i32)
  return
}

// -----
// CHECK-LABEL: succeededOilistCustom
// CHECK-SAME: (%[[ARG0:.*]]: i32, %[[ARG1:.*]]: i32, %[[ARG2:.*]]: i32)
func.func @succeededOilistCustom(%arg0: i32, %arg1: i32, %arg2: i32) {
  // CHECK: test.oilist_custom private(%[[ARG0]], %[[ARG1]] : i32, i32)
  test.oilist_custom private (%arg0, %arg1 : i32, i32)
  // CHECK: test.oilist_custom private(%[[ARG0]], %[[ARG1]] : i32, i32) nowait
  test.oilist_custom private (%arg0, %arg1 : i32, i32) nowait
  // CHECK: test.oilist_custom private(%arg0, %arg1 : i32, i32) reduction (%arg1) nowait
  test.oilist_custom nowait reduction (%arg1) private (%arg0, %arg1 : i32, i32)
  return
}

// -----

func.func @failedHasDominanceScopeOutsideDominanceFreeScope() -> () {
  "test.ssacfg_region"() ({
    test.graph_region {
      // expected-error @+1 {{operand #0 does not dominate this use}}
      %2:3 = "bar"(%1) : (i64) -> (i1,i1,i1)
    }
    // expected-note @+1 {{operand defined here}}
    %1 = "baz"() : () -> (i64)
  }) : () -> ()
  return
}

// -----

// Ensure that SSACFG regions of operations in GRAPH regions are
// checked for dominance
func.func @illegalInsideDominanceFreeScope() -> () {
  test.graph_region {
    func.func @test() -> i1 {
    ^bb1:
      // expected-error @+1 {{operand #0 does not dominate this use}}
      %2:3 = "bar"(%1) : (i64) -> (i1,i1,i1)
      // expected-note @+1 {{operand defined here}}
	   %1 = "baz"(%2#0) : (i1) -> (i64)
      return %2#1 : i1
    }
    "terminator"() : () -> ()
  }
  return
}

// -----

// Ensure that SSACFG regions of operations in GRAPH regions are
// checked for dominance
func.func @illegalCDFGInsideDominanceFreeScope() -> () {
  test.graph_region {
    func.func @test() -> i1 {
    ^bb1:
      // expected-error @+1 {{operand #0 does not dominate this use}}
      %2:3 = "bar"(%1) : (i64) -> (i1,i1,i1)
      cf.br ^bb4
    ^bb2:
      cf.br ^bb2
    ^bb4:
      %1 = "foo"() : ()->i64   // expected-note {{operand defined here}}
		return %2#1 : i1
    }
     "terminator"() : () -> ()
  }
  return
}

// -----

// Ensure that GRAPH regions still have all values defined somewhere.
func.func @illegalCDFGInsideDominanceFreeScope() -> () {
  test.graph_region {
    // expected-error @+1 {{use of undeclared SSA value name}}
    %2:3 = "bar"(%1) : (i64) -> (i1,i1,i1)
    "terminator"() : () -> ()
  }
  return
}

// -----

func.func @graph_region_cant_have_blocks() {
  test.graph_region {
    // expected-error@-1 {{'test.graph_region' op expects graph region #0 to have 0 or 1 blocks}}
  ^bb42:
    cf.br ^bb43
  ^bb43:
    "terminator"() : () -> ()
  }
}

// -----

// Check that we can query traits in types
func.func @succeeded_type_traits() {
  // CHECK: "test.result_type_with_trait"() : () -> !test.test_type_with_trait
  "test.result_type_with_trait"() : () -> !test.test_type_with_trait
  return
}

// -----

// Check that we can query traits in types
func.func @failed_type_traits() {
  // expected-error@+1 {{result type should have trait 'TestTypeTrait'}}
  "test.result_type_with_trait"() : () -> i32
  return
}

// -----

// Check that we can query traits in attributes
func.func @succeeded_attr_traits() {
  // CHECK: "test.attr_with_trait"() <{attr = #test.attr_with_trait}> : () -> ()
  "test.attr_with_trait"() {attr = #test.attr_with_trait} : () -> ()
  return
}

// -----

// Check that we can query traits in attributes
func.func @failed_attr_traits() {
  // expected-error@+1 {{'attr' attribute should have trait 'TestAttrTrait'}}
  "test.attr_with_trait"() {attr = 42 : i32} : () -> ()
  return
}
