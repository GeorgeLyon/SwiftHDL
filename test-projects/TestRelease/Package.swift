// swift-tools-version: 5.9

import Foundation
import PackageDescription

let releaseString = ProcessInfo.processInfo.environment["SWIFTHDL_RELEASE"]!
let release = Version(releaseString)!

let package = Package(
  name: "TestRelease",
  products: [
    .library(
      name: "TestRelease",
      targets: ["TestRelease"])
  ],
  dependencies: [
    .package(url: "https://github.com/GeorgeLyon/SwiftHDL", exact: release)
  ],
  targets: [
    .target(
      name: "TestRelease",
      dependencies: ["SwiftHDL"]),
    .testTarget(
      name: "TestReleaseTests",
      dependencies: ["TestRelease"]),
  ]
)
