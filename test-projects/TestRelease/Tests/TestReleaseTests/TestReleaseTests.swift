import XCTest

@testable import TestRelease

final class TestReleaseTests: XCTestCase {
  func testExample() throws {
    print(TestRelease.testText)
    XCTAssertEqual(TestRelease.testText, "Hello, CIRCT!")
  }
}
