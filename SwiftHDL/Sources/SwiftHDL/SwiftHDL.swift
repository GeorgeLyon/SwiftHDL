@_implementationOnly import CIRCTFirtool

public enum SwiftHDL {
  public static func test() -> String {
    let context = mlir.MLIRContextCreate();
    let attribute = mlir.StringAttr.get(context, llvm.Twine("Hello, CIRCT!"))
    return String(attribute.str())
  }
}
