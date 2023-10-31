// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "SwiftHDL",
  products: [
    .library(
      name: "SwiftHDL",
      targets: ["SwiftHDL"]),
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
