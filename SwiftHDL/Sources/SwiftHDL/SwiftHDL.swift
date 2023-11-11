@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

public enum SwiftHDL {
  public static func test() -> String {
    let threadPool = ThreadPool()
    let context = MLIRContext(threadPool)
    let attribute = context.withCxx { context in
      return mlir.StringAttr.get(context, "Hello, CIRCT!")
    }
    
    return String(attribute.str())
  }
}
