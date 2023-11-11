// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "SwiftHDL",
  platforms: [
    .macOS(.v10_13)
  ],
  products: [
    .library(
      name: "SwiftHDL",
      type: .dynamic,
      targets: ["SwiftHDL"])
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
      dependencies: ["SwiftHDL"],
      swiftSettings: [
        .interoperabilityMode(.Cxx)
      ]),
  ],
  cxxLanguageStandard: .cxx17
)
