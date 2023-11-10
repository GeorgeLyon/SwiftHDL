@_implementationOnly import CxxSwiftHDL

public enum SwiftHDL {
  public static func test() -> String {
    let context = mlir.MLIRContextCreate()
    let string: StaticString = "Hello, CIRCT!"
    let attribute = string.withUTF8Buffer { buffer in
      let stringRef = llvm.StringRef(buffer.baseAddress, buffer.count)
      return mlir.StringAttr.get(context, llvm.Twine(stringRef))
    }
    return String(attribute.str())
  }
}
