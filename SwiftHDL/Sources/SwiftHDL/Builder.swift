@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

public protocol Op {
  static func getOperationName() -> llvm.StringLiteral
}

public func foo<Operation: Op>(_ op: Operation.Type) {
  let x = Operation.getOperationName()
}

public class ManagedBuilder: Identifiable {
  public init(_ context: ManagedMLIRContext) async {
    self.context = context
    self.id = await context.builderNamespace.next()
    self.cxx = context.withCxx(CxxSwiftHDL.createOpBuilder)
  }

  public let context: ManagedMLIRContext
  public let id: ID
  public var isValid: Bool = true
  public var cxx: mlir.OpBuilder

  public func build<Opx: Op, each Arg>(
    _ fn: (mlir.OpBuilder, mlir.OpState) -> Opx,
    _ location: mlir.Location,
    _ args: repeat each Arg
  ) -> Opx {
    let foo = circt.firrtl.CircuitOp.getOperationName()
    // let state = mlir.OperationState(location)
    fatalError()
  }
}
