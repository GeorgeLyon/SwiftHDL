// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "SwiftHDLExample",
  products: [
    .library(
      name: "SwiftHDLExample",
      targets: ["SwiftHDLExample"])
  ],
  targets: [
    .binaryTarget(
      name: "SwiftHDL",
      path: "../build/xcodebuild/SwiftHDL.xcframework"),
    .target(
      name: "SwiftHDLExample",
      dependencies: ["SwiftHDL"]),
    .testTarget(
      name: "SwiftHDLExampleTests",
      dependencies: ["SwiftHDLExample"]),
  ]
)
