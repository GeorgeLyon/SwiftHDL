// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "TestXCFramework",
  products: [
    .library(
      name: "TestXCFramework",
      targets: ["TestXCFramework"])
  ],
  dependencies: [],
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
