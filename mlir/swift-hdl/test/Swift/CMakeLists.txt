set(CMAKE_Swift_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/swift)
enable_language(Swift)

add_executable(SwiftHDL_Swift_Test
  Test.swift
)
target_link_libraries(SwiftHDL_Swift_Test SwiftHDL)
target_compile_options(SwiftHDL_Swift_Test
  PRIVATE
    -cxx-interoperability-mode=default
    -Xcc -std=c++17)
