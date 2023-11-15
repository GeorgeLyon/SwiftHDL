import XCTest

@testable import SwiftHDL

final class SwiftHDLTests: XCTestCase {
  func testExample() async throws {
    let text = await SwiftHDL.test()
    print(text)
    XCTAssertEqual(text, "Hello, CIRCT!")
  }
}
