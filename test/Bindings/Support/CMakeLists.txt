add_swift_hdl_executable(swift-hdl-bindings-support-test
  Test.cpp)
target_link_libraries(swift-hdl-bindings-support-test
  PRIVATE
  SwiftHDLBindingsSupport)
