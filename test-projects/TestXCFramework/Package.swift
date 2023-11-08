// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "TestXCFramework",
  products: [
    .library(
      name: "TestXCFramework",
      targets: ["TestXCFramework"])
  ],
  dependencies: [
    .package(url: "https://github.com/apple/swift-format", branch: "release/5.9")
  ],
  targets: [
    .binaryTarget(
      name: "SwiftHDL",
      path: "../../build/xcodebuild/SwiftHDL.xcframework"),
    .target(
      name: "TestXCFramework",
      dependencies: ["SwiftHDL"]),
    .testTarget(
      name: "TestXCFrameworkTests",
      dependencies: ["TestXCFramework"]),
  ]
)
