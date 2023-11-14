@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

// MARK: Context

/// A class which manages the lifespan of MLIRContext. This class is not threadsafe.
public class ManagedMLIRContext: Identifiable {
  public init(_ threadPool: ThreadPool) async {
    self.threadPool = threadPool
    cxx = threadPool.withCxx { cxxThreadPool in
      CxxSwiftHDL.createMLIRContext(cxxThreadPool)
    }
    id = await ID.namespace.next()
    builderNamespace = ManagedBuilder.ID.Namespace(contextID: id)
  }
  deinit {
    CxxSwiftHDL.destroyMLIRContext(cxx)
  }

  public let id: ID
  let builderNamespace: ManagedBuilder.ID.Namespace

  func withCxx<T>(_ body: (OpaquePointer) throws -> T) rethrows -> T {
    return try body(cxx)
  }

  private let threadPool: ThreadPool
  private let cxx: OpaquePointer
}

// MARK: IDs

/// SwiftHDL enables direct manipulation of the underlying MLIR data structures by an end-user in Swift. SwiftHDL strives to make such interactions safe, and avoid undefined behavior (like dereferenceing a pointer that is no longer valid). To achieve this, SwiftHDL makes heavy use of generational references, tying all values that may become invalid to a context or builder ID.
extension ManagedMLIRContext {
  public struct ID: Hashable {
    private typealias RawValue = UInt16
    private let value: RawValue

    fileprivate actor Namespace {
      func next() -> ID {
        let value = _nextValue
        // Swift integers abort on overflow
        _nextValue += 1
        return ID(value: value)
      }
      private var _nextValue: RawValue = 0
    }
    fileprivate static let namespace = Namespace()
  }
}

extension ManagedBuilder {
  public struct ID: Hashable {
    private let contextID: ManagedMLIRContext.ID
    private typealias RawValue = UInt16
    private let value: RawValue

    actor Namespace {
      init(contextID: ManagedMLIRContext.ID) {
        self.contextID = contextID
      }
      let contextID: ManagedMLIRContext.ID

      func next() -> ID {
        let value = _nextValue
        // Swift integers abort on overflow
        _nextValue += 1
        return ID(contextID: contextID, value: value)
      }
      private var _nextValue: RawValue = 0
    }
  }
}

// MARK: SwiftHDL

extension ManagedMLIRContext {
  public func loadSwiftHDLDialects() {
    withCxx { cxxContext in
      CxxSwiftHDL.loadSwiftHDLDialects(cxxContext)
    }
  }
}
