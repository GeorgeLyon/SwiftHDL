@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

public protocol Op {
  static func getOperationName() -> llvm.StringLiteral
}

public func foo<Operation: Op>(_ op: Operation.Type) {
  print(String(Operation.getOperationName().str()))
}

// extension circt.firrtl.CircuitOp: Op {

// }

public enum SwiftHDL {
  private static let threadPool = ManagedThreadPool()

  public static func test() async -> String {
    let context = await ManagedMLIRContext(threadPool)
    let attribute = context.withCxx { context in
      return mlir.StringAttr.get(context, "Hello, CIRCT!")
    }

    let name = circt.firrtl.CircuitOp.getOperationName()
    print(String(name.str()))

    // foo(circt.firrtl.CircuitOp.self)

    return String(attribute.str())
  }
}
