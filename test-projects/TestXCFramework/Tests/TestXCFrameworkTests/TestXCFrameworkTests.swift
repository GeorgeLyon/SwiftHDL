import XCTest

@testable import TestXCFramework

final class TestXCFrameworkTests: XCTestCase {
  func testExample() throws {
    XCTAssertEqual(TestXCFramework.testText, "Hello, CIRCT!")
  }
}
