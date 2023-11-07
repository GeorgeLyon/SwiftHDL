// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "SwiftHDL",
  platforms: [
    .macOS(.v14)
  ],
  products: [
    .library(
      name: "SwiftHDL",
      targets: ["SwiftHDL"])
  ],
  dependencies: [
    .package(url: "https://github.com/apple/swift-format", branch: "release/5.9")
  ],
  targets: [
    .systemLibrary(
      name: "CxxSwiftHDL",
      pkgConfig: "SwiftHDL"),
    .target(
      name: "SwiftHDL",
      dependencies: ["CxxSwiftHDL"],
      swiftSettings: [
        .interoperabilityMode(.Cxx)
      ]),
    .testTarget(
      name: "SwiftHDLTests",
      dependencies: ["SwiftHDL"]),
  ],
  cxxLanguageStandard: .cxx17
)
