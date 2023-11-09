// swift-tools-version: 5.9

import PackageDescription

let package = Package(
  name: "SwiftHDL",
  products: [
    .library(
      name: "SwiftHDL",
      targets: ["SwiftHDL"])
  ],
  dependencies: [],
  targets: [
    .binaryTarget(
      name: "SwiftHDL",
      url: "https://github.com/GeorgeLyon/SwiftHDL/releases/download/0.0.1/SwiftHDL.zip",
      checksum: "c97ee27469dc5650d67fe882f3a9730b99ac323e01fcef5bbe6cbe445312ac22")
  ]
)
