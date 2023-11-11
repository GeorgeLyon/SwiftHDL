import XCTest

@testable import SwiftHDL

final class SwiftHDLTests: XCTestCase {
  func testExample() throws {
    print(SwiftHDL.test())
    XCTAssertEqual(SwiftHDL.test(), "Hello, CIRCT!")
  }
}
