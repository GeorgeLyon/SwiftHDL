add_swift_hdl_executable(swift-hdl-placeholder-test
  PlaceholderTest.cpp)
target_link_libraries(swift-hdl-placeholder-test
  PRIVATE
  SwiftHDLPlaceholder)
