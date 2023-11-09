// swift-tools-version: 5.9

import Foundation
import PackageDescription

let swiftHDLTarget: Target
do {
  let env = ProcessInfo.processInfo.environment
  let pathOption = env["SWIFTHDL_BINARY_TARGET_PATH"]
  let urlOption = env["SWIFTHDL_BINARY_TARGET_URL"]
  let checksumOption = env["SWIFTHDL_BINARY_TARGET_CHECKSUM"]
  switch (pathOption, urlOption, checksumOption) {
  // Use the default path to the binary created by `create-xcframework`
  case (.none, .none, .none):
    swiftHDLTarget = .binaryTarget(
      name: "SwiftHDL",
      path: "../../build/xcodebuild/SwiftHDL.xcframework")
  // Specify a custom binary path
  case (.some(let path), .none, .none):
    swiftHDLTarget = .binaryTarget(
      name: "SwiftHDL",
      path: path)
  // Specify a custom url and checksum
  case (.none, .some(let url), .some(let checksum)):
    swiftHDLTarget = .binaryTarget(
      name: "SwiftHDL",
      url: url,
      checksum: checksum)
  // Invalid configurations
  case (_, .some, .none), (_, .none, .some):
    fatalError("Must specify both url and checksum.")
  case (.some, .some, .some):
    fatalError("Cannot specify both path and url/checksum.")
  }
}

let package = Package(
  name: "TestXCFramework",
  products: [
    .library(
      name: "TestXCFramework",
      targets: ["TestXCFramework"])
  ],
  dependencies: [],
  targets: [
    swiftHDLTarget,
    .target(
      name: "TestXCFramework",
      dependencies: ["SwiftHDL"]),
    .testTarget(
      name: "TestXCFrameworkTests",
      dependencies: ["TestXCFramework"]),
  ]
)
