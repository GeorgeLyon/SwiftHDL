set(CMAKE_Swift_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/swift)
enable_language(Swift)

add_executable(SwiftHDL_Bindings_Swift_Test
  Test.swift
)
target_link_libraries(SwiftHDL_Bindings_Swift_Test SwiftHDLBindingsSupport)
target_compile_options(SwiftHDL_Bindings_Swift_Test
  PRIVATE
    # -emit-module
    "SHELL:-cxx-interoperability-mode=swift-5.9"
    "SHELL:-Xfrontend -validate-tbd-against-ir=none"
    -Xcc -std=c++17)
