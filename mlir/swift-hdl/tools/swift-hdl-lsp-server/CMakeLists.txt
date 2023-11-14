add_swift_hdl_tool(swift-hdl-lsp-server
  swift-hdl-lsp-server.cpp)
target_link_libraries(swift-hdl-lsp-server PRIVATE 
  CIRCTFIRRTL
  MLIRLspServerLib)
llvm_update_compile_flags(swift-hdl-lsp-server)

mlir_check_all_link_libraries(swift-hdl-lsp-server)
