@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

public class ManagedThreadPool {
  public init() {
    cxx = CxxSwiftHDL.createThreadPool()
  }
  deinit {
    CxxSwiftHDL.destroyThreadPool(cxx)
  }
  func withCxx<T>(_ body: (OpaquePointer) throws -> T) rethrows -> T {
    return try body(cxx)
  }
  private let cxx: OpaquePointer
}
