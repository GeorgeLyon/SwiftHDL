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
      type: .dynamic,
      targets: ["SwiftHDL"]),
  ],
  dependencies: [
  ],
  targets: [
    .systemLibrary(
      name: "CIRCTFirtool", 
      pkgConfig: "CIRCTFirtool"),
    .target(
      name: "SwiftHDL",
      dependencies: ["CIRCTFirtool"],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]),
    .testTarget(
      name: "SwiftHDLTests",
      dependencies: ["SwiftHDL"]),
  ],
  cxxLanguageStandard: .cxx17
)
