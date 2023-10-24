# Generate a list of CMake library targets so that other CMake projects can
# link against them.
set(SWIFT_HDL_INSTALL_PACKAGE_DIR lib${LLVM_LIBDIR_SUFFIX}/cmake/swift-hdl)
set(swift_hdl_cmake_builddir "${CMAKE_BINARY_DIR}/${SWIFT_HDL_INSTALL_PACKAGE_DIR}")

# Keep this in sync with mlir/cmake/CMakeLists.txt!
set(MLIR_INSTALL_PACKAGE_DIR lib${LLVM_LIBDIR_SUFFIX}/cmake/mlir)
set(mlir_cmake_builddir "${MLIR_BINARY_DIR}/${MLIR_INSTALL_PACKAGE_DIR}")

# Keep this in sync with llvm/cmake/CMakeLists.txt!
set(LLVM_INSTALL_PACKAGE_DIR lib${LLVM_LIBDIR_SUFFIX}/cmake/llvm)
set(llvm_cmake_builddir "${LLVM_BINARY_DIR}/${LLVM_INSTALL_PACKAGE_DIR}")

# Generate a list of CMake library targets so that other CMake projects can
# link against them. LLVM calls its version of this file LLVMExports.cmake, but
# the usual CMake convention seems to be ${Project}Targets.cmake.
get_property(SWIFT_HDL_EXPORTS GLOBAL PROPERTY SWIFT_HDL_EXPORTS)
export(TARGETS ${SWIFT_HDL_EXPORTS} FILE ${swift_hdl_cmake_builddir}/SwiftHDLTargets.cmake)

get_property(SWIFT_HDL_ALL_LIBS GLOBAL PROPERTY SWIFT_HDL_ALL_LIBS)
get_property(SWIFT_HDL_DIALECT_LIBS GLOBAL PROPERTY SWIFT_HDL_DIALECT_LIBS)
get_property(SWIFT_HDL_CONVERSION_LIBS GLOBAL PROPERTY SWIFT_HDL_CONVERSION_LIBS)
get_property(SWIFT_HDL_TRANSLATION_LIBS GLOBAL PROPERTY SWIFT_HDL_TRANSLATION_LIBS)
get_property(SWIFT_HDL_ANALYSIS_LIBS GLOBAL PROPERTY SWIFT_HDL_ANALYSIS_LIBS)
get_property(SWIFT_HDL_VERIFICATION_LIBS GLOBAL PROPERTY SWIFT_HDL_VERIFICATION_LIBS)

# Generate SwiftHDLConfig.cmake for the build tree.
set(SWIFT_HDL_CONFIG_CMAKE_DIR "${swift_hdl_cmake_builddir}")
set(SWIFT_HDL_CONFIG_LLVM_CMAKE_DIR "${llvm_cmake_builddir}")
set(SWIFT_HDL_CONFIG_MLIR_CMAKE_DIR "${mlir_cmake_builddir}")
set(SWIFT_HDL_CONFIG_LIBRARY_DIRS "${SWIFT_HDL_LIBRARY_DIR}")
set(SWIFT_HDL_CONFIG_BINARY_DIR "${SWIFT_HDL_BINARY_DIR}")
set(SWIFT_HDL_CONFIG_TOOLS_DIR "${SWIFT_HDL_TOOLS_DIR}")
set(SWIFT_HDL_CONFIG_INCLUDE_EXPORTS "include(\"\${SWIFT_HDL_CMAKE_DIR}/SwiftHDLTargets.cmake\")")
set(SWIFT_HDL_CONFIG_INCLUDE_DIRS
  "${SWIFT_HDL_SOURCE_DIR}/include"
  "${SWIFT_HDL_BINARY_DIR}/include"
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/SwiftHDLConfig.cmake.in
  ${swift_hdl_cmake_builddir}/SwiftHDLConfig.cmake
  @ONLY)
set(SWIFT_HDL_CONFIG_CMAKE_DIR)
set(SWIFT_HDL_CONFIG_LLVM_CMAKE_DIR)
set(SWIFT_HDL_CONFIG_MLIR_CMAKE_DIR)
set(SWIFT_HDL_CONFIG_LIBRARY_DIRS)
set(SWIFT_HDL_CONFIG_BINARY_DIR)
set(SWIFT_HDL_CONFIG_TOOLS_DIR)
set(SWIFT_HDL_CONFIG_INCLUDE_EXPORTS)
set(SWIFT_HDL_CONFIG_INCLUDE_DIRS)

# Generate SwiftHDLConfig.cmake for the install tree.
set(SWIFT_HDL_CONFIG_CODE "
# Compute the installation prefix from this SwiftHDLConfig.cmake file location.
get_filename_component(SWIFT_HDL_INSTALL_PREFIX \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)")
# Construct the proper number of get_filename_component(... PATH)
# calls to compute the installation prefix.
string(REGEX REPLACE "/" ";" _count "${SWIFT_HDL_INSTALL_PACKAGE_DIR}")
foreach(p ${_count})
  set(SWIFT_HDL_CONFIG_CODE "${SWIFT_HDL_CONFIG_CODE}
get_filename_component(SWIFT_HDL_INSTALL_PREFIX \"\${SWIFT_HDL_INSTALL_PREFIX}\" PATH)")
endforeach(p)
set(SWIFT_HDL_CONFIG_CMAKE_DIR "\${SWIFT_HDL_INSTALL_PREFIX}/${SWIFT_HDL_INSTALL_PACKAGE_DIR}")
set(SWIFT_HDL_CONFIG_LLVM_CMAKE_DIR "\${SWIFT_HDL_INSTALL_PREFIX}/${LLVM_INSTALL_PACKAGE_DIR}")
set(SWIFT_HDL_CONFIG_MLIR_CMAKE_DIR "\${SWIFT_HDL_INSTALL_PREFIX}/${MLIR_INSTALL_PACKAGE_DIR}")
set(SWIFT_HDL_CONFIG_LIBRARY_DIRS "\${SWIFT_HDL_INSTALL_PREFIX}/lib")
set(SWIFT_HDL_CONFIG_BINARY_DIR "\${SWIFT_HDL_INSTALL_PREFIX}")
set(SWIFT_HDL_CONFIG_TOOLS_DIR "\${SWIFT_HDL_INSTALL_PREFIX}/bin")
set(SWIFT_HDL_CONFIG_INCLUDE_EXPORTS "include(\"\${SWIFT_HDL_CMAKE_DIR}/SwiftHDLTargets.cmake\")")
set(SWIFT_HDL_CONFIG_INCLUDE_DIRS
  "\${SWIFT_HDL_INSTALL_PREFIX}/include"
  )
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SwiftHDLConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SwiftHDLConfig.cmake
    @ONLY)
set(SWIFT_HDL_CONFIG_CODE)
set(SWIFT_HDL_CONFIG_CMAKE_DIR)
set(SWIFT_HDL_CONFIG_LLVM_CMAKE_DIR)
set(SWIFT_HDL_CONFIG_MLIR_CMAKE_DIR)
set(SWIFT_HDL_CONFIG_LIBRARY_DIRS)
set(SWIFT_HDL_CONFIG_BINARY_DIR)
set(SWIFT_HDL_CONFIG_TOOLS_DIR)
set(SWIFT_HDL_CONFIG_INCLUDE_EXPORTS)
set(SWIFT_HDL_CONFIG_INCLUDE_DIRS)

if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
  # Not TOOLCHAIN ONLY, so install the SwiftHDL parts as well
  # Include the cmake files so other tools can use swift-hdl-tblgen, etc.
  install(EXPORT SwiftHDLTargets DESTINATION ${SWIFT_HDL_INSTALL_PACKAGE_DIR}
          COMPONENT swift-hdl-cmake-exports)

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SwiftHDLConfig.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/AddSwiftHDL.cmake
    DESTINATION ${SWIFT_HDL_INSTALL_PACKAGE_DIR}
    COMPONENT swift-hdl-cmake-exports)

  if(NOT LLVM_ENABLE_IDE)
    # Add a dummy target so this can be used with LLVM_DISTRIBUTION_COMPONENTS
    add_custom_target(swift-hdl-cmake-exports)
    add_llvm_install_targets(install-swift-hdl-cmake-exports
                             COMPONENT swift-hdl-cmake-exports)
  endif()
endif()
