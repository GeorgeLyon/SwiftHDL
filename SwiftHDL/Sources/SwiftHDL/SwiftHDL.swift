@_implementationOnly import CxxSwiftHDL

public enum SwiftHDL {
  public static func test() -> String {
    let context = mlir.MLIRContextCreate()
    var string = "Hello, CIRCT!"
    let attribute = string.withUTF8 { buffer in
      let stringRef = llvm.StringRef(buffer.baseAddress, buffer.count)
      return mlir.StringAttr.get(context, llvm.Twine(stringRef))
    }
    return String(attribute.str())
  }
}
