add_subdirectory(Placeholder)
add_subdirectory(Swift)

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  )

add_lit_testsuite(check-swift-hdl "Running the SwiftHDL regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS 
    FileCheck count not
    swift-hdl-test
    SwiftHDL_Swift_Test
  )
set_target_properties(check-swift-hdl PROPERTIES FOLDER "Tests")
