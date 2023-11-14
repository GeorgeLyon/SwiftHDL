@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

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

  public func build<Op, each Arg>(
    _ fn: (mlir.OpBuilder, mlir.OpState) -> Op,
    _ location: mlir.Location,
    _ args: repeat each Arg
  ) -> Op {
    let state = mlir.OperationState(location)
    fatalError()
  }
}
