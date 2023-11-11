@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

extension llvm.StringRef: ExpressibleByStringLiteral {
  public init(stringLiteral value: StaticString) {
    self = value.withUTF8Buffer { buffer in
      Self(buffer.baseAddress, buffer.count)
    }
  }
}

extension llvm.Twine: ExpressibleByStringLiteral {
  public init(stringLiteral value: StaticString) {
    self.init(llvm.StringRef(stringLiteral: value))
  }
}
