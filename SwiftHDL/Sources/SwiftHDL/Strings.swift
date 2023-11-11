@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

extension llvm.StringRef: ExpressibleByStringLiteral {
  public init(stringLiteral value: StaticString) {
    self.init(value.utf8Start, value.utf8CodeUnitCount)
  }
}

extension llvm.Twine: ExpressibleByStringLiteral {
  public init(stringLiteral value: StaticString) {
    self.init(llvm.StringRef(stringLiteral: value))
  }
}
