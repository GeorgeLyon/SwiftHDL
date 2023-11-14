@_implementationOnly import CxxStdlib
@_implementationOnly import CxxSwiftHDL

public class ManagedBuilder: Identifiable {
  public init(_ context: ManagedMLIRContext) async {
    self.context = context
    self.id = await context.builderNamespace.next()
  }

  public let context: ManagedMLIRContext
  public let id: ID
  public var isValid: Bool = true
}
