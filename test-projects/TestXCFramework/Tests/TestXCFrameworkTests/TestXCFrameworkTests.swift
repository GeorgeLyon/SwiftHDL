import XCTest

@testable import TestXCFramework

final class TestXCFrameworkTests: XCTestCase {
  func testExample() throws {
    print(TestXCFramework.testText)
    XCTAssertEqual(TestXCFramework.testText, "Hello, CIRCT!")
  }
}
