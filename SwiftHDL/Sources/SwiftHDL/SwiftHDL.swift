@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

// extension circt.firrtl.CircuitOp: Op {

// }

public enum SwiftHDL {
  private static let threadPool = ManagedThreadPool()

  public static func test() async -> String {
    let context = await ManagedMLIRContext(threadPool)
    let attribute = context.withCxx { context in
      return mlir.StringAttr.get(context, "Hello, CIRCT!")
    }
    return String(attribute.str())
  }

  public static func gcd() async {
    let context = await ManagedMLIRContext(threadPool)
    context.loadSwiftHDLDialects()
    let builder = await ManagedBuilder(context)

    // foo(circt.firrtl.CircuitOp.self)

  }
}
