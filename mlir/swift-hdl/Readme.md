# SwiftHDL CMake Project

Originally when starting to develop SwiftHDL, I had thought that there would be a C++ "Bridge" layer to expose a Swift-friendly CIRCT C++ API. As I played around with Swift's C++ interop functionality more and more, I began to think that it would be possible to write the entire project in Swift, and just export the CIRCT C++ API as-is (props to Swift's C++ interop team!). While this is still the plan-of-record, I did have a nicely set-up project for the C++ bridge layer so I'm leaving it in the repo, both for reference and if I run into some issue that I can't solve on the Swift side.

As an added bonus, this project contains `swift-hdl-lsp-server`, which is like `circt-lsp-server` but only supports SwiftHDL-related dialects. This allows us to build the LSP server without building a bunch of stuff we don't use in CIRCT.

