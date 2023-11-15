add_swift_hdl_executable(swift-hdl-test
  SwiftHDLTest.cpp)
target_link_libraries(swift-hdl-test
  PRIVATE
  SwiftHDL)
