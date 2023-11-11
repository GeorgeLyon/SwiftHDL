@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

class MLIRContext {
  init(_ threadPool: ThreadPool) {
    self.threadPool = threadPool
    cxx = threadPool.withCxx(CxxSwiftHDL.createMLIRContext)!
    CxxSwiftHDL.loadSwiftHDLDialects(cxx)
  }
  deinit {
    CxxSwiftHDL.destroyMLIRContext(cxx)
  }
  func withCxx<T>(_ body: (OpaquePointer) throws -> T) rethrows -> T {
    return try body(cxx)
  }
  private let threadPool: ThreadPool
  private let cxx: OpaquePointer
}
