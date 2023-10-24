##===- CMakeLists.txt - SwiftHDL cmake root -------------------*- cmake -*-===//
##
## Configure the SwiftHDL build.
##
##===----------------------------------------------------------------------===//

cmake_minimum_required(VERSION 3.13.4)

if(POLICY CMP0068)
  cmake_policy(SET CMP0068 NEW)
  set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
endif()

if(POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

# CMP0116: Ninja generators transform `DEPFILE`s from `add_custom_command()`
# New in CMake 3.20. https://cmake.org/cmake/help/latest/policy/CMP0116.html
if(POLICY CMP0116)
  cmake_policy(SET CMP0116 OLD)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# If we are not building as a part of LLVM, build SwiftHDL as an
# standalone project, using LLVM as an external library:
if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )

# Generate a CompilationDatabase (compile_commands.json file) for our build,
# for use by clang_complete, YouCompleteMe, etc.
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#-------------------------------------------------------------------------------
# Project setup and globals
#-------------------------------------------------------------------------------
  project(swift-hdl LANGUAGES CXX C)

#-------------------------------------------------------------------------------
# Options and settings
#-------------------------------------------------------------------------------

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c- /GR-")
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
  endif ()

#-------------------------------------------------------------------------------
# CIRCT/MLIR/LLVM Configuration
#-------------------------------------------------------------------------------

  find_package(MLIR REQUIRED CONFIG)

  message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)

  list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

  include(TableGen)
  include(AddLLVM)
  include(AddMLIR)
  include(HandleLLVMOptions)

  set(SWIFT_HDL_BUILT_STANDALONE 1)
  set(BACKEND_PACKAGE_STRING "LLVM ${LLVM_PACKAGE_VERSION}")

  set(MLIR_JNI_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
else()
  # SwiftHDL is being built as an LLVM_EXTERNAL_PROJECT

  # -- MLIR Configuration

  set(MLIR_MAIN_SRC_DIR ${LLVM_MAIN_SRC_DIR}/../mlir ) # --src-root
  set(MLIR_INCLUDE_DIR ${MLIR_MAIN_SRC_DIR}/include ) # --includedir
  set(MLIR_TABLEGEN_OUTPUT_DIR ${LLVM_BINARY_DIR}/tools/mlir/include)
  set(MLIR_TABLEGEN_EXE $<TARGET_FILE:mlir-tblgen>)
  include_directories(SYSTEM ${MLIR_INCLUDE_DIR})
  include_directories(SYSTEM ${MLIR_TABLEGEN_OUTPUT_DIR})

  set(BACKEND_PACKAGE_STRING "${PACKAGE_STRING}")

  set(MLIR_JNI_LIB_DIR "${LLVM_BINARY_DIR}/lib")

  # -- CIRCT Configuration

  set(CIRCT_MAIN_SRC_DIR ${LLVM_EXTERNAL_CIRCT_SOURCE_DIR})
  set(CIRCT_INCLUDE_DIR ${CIRCT_MAIN_SRC_DIR}/include)
  set(CIRCT_TABLEGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/tools/circt/include)
  include_directories(SYSTEM ${CIRCT_INCLUDE_DIR})
  include_directories(SYSTEM ${CIRCT_TABLEGEN_OUTPUT_DIR})
endif()

# Define the default arguments to use with 'lit', and an option for the user to
# override.
set(LIT_ARGS_DEFAULT "-sv")
if (MSVC_IDE OR XCODE)
  set(LIT_ARGS_DEFAULT "${LIT_ARGS_DEFAULT} --no-progress-bar")
endif()
set(LLVM_LIT_ARGS "${LIT_ARGS_DEFAULT}" CACHE STRING "Default options for lit")

#-------------------------------------------------------------------------------
# SwiftHDL configuration
#-------------------------------------------------------------------------------

# SwiftHDL defines.
set(SWIFT_HDL_MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} ) # --src-root
set(SWIFT_HDL_MAIN_INCLUDE_DIR ${SWIFT_HDL_MAIN_SRC_DIR}/include)

set(SWIFT_HDL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWIFT_HDL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SWIFT_HDL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
set(SWIFT_HDL_LIBRARY_DIR ${CMAKE_BINARY_DIR}/lib)
set(SWIFT_HDL_TOOLS_DIR ${CMAKE_BINARY_DIR}/bin)
set(SWIFT_HDL_UTILS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utils)

option(SWIFT_HDL_INCLUDE_TOOLS "Generate build targets for the SwiftHDL tools." ON)
option(SWIFT_HDL_BUILD_TOOLS "Build the SwiftHDL tools. If OFF, just generate build targets." ON)
set(SWIFT_HDL_TOOLS_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}" CACHE PATH
    "Path for binary subdirectory (defaults to '${CMAKE_INSTALL_BINDIR}')")

list(APPEND CMAKE_MODULE_PATH "${MLIR_MAIN_SRC_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(AddSwiftHDL)

# Installing the headers and docs needs to depend on generating any public
# tablegen'd targets.
add_custom_target(swift-hdl-headers)
set_target_properties(swift-hdl-headers PROPERTIES FOLDER "Misc")
add_custom_target(swift-hdl-doc)
    
# Add MLIR and LLVM headers to the include path
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})

# Add SwiftHDL files to the include path
include_directories(${SWIFT_HDL_MAIN_INCLUDE_DIR})
include_directories(${SWIFT_HDL_INCLUDE_DIR})

# Set the release tag.
option(SWIFT_HDL_RELEASE_TAG_ENABLED "Emit the release tag to output." OFF)
if (NOT SWIFT_HDL_RELEASE_TAG_ENABLED)
  message(STATUS "Version generation is disabled. To enable the version "
               "generation, please set SWIFT_HDL_RELEASE_TAG_ENABLED CMake "
               "variable")
endif()

set(SWIFT_HDL_RELEASE_TAG "swift-hdl" CACHE STRING
    "Prefix of the release tag")

#-------------------------------------------------------------------------------
# Directory setup
#-------------------------------------------------------------------------------

add_subdirectory(lib)
if(SWIFT_HDL_INCLUDE_TOOLS)
  add_subdirectory(tools)
endif()
add_subdirectory(test)

option(SWIFT_HDL_INCLUDE_DOCS "Generate build targets for the SwiftHDL docs.")
if (SWIFT_HDL_INCLUDE_DOCS)
  add_subdirectory(docs)
endif()

install(DIRECTORY include/swift-hdl
  DESTINATION include
  COMPONENT swift-headers
  FILES_MATCHING
  PATTERN "*.def"
  PATTERN "*.h"
  PATTERN "*.inc"
  PATTERN "*.td"
  PATTERN "*.sv"
  PATTERN "LICENSE.TXT"
  )

install(DIRECTORY ${SWIFT_HDL_INCLUDE_DIR}/swift-hdl
  DESTINATION include
  COMPONENT swift-hdl-headers
  FILES_MATCHING
  PATTERN "*.def"
  PATTERN "*.h"
  PATTERN "*.gen"
  PATTERN "*.inc"
  PATTERN "*.td"
  PATTERN "CMakeFiles" EXCLUDE
  PATTERN "config.h" EXCLUDE
  )

if (NOT LLVM_ENABLE_IDE)
  add_llvm_install_targets(install-swift-hdl-headers
                           DEPENDS swift-hdl-headers
                           COMPONENT swift-hdl-headers)
endif()

add_subdirectory(cmake/modules)

# Set RPATH to $ORIGIN on all targets.
function(set_rpath_all_targets dir)
  get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
  foreach(subdir ${subdirectories})
    set_rpath_all_targets(${subdir})
  endforeach()

  get_directory_property(LCL_TARGETS DIRECTORY ${dir} BUILDSYSTEM_TARGETS)
  set_property(TARGET ${LCL_TARGETS} PROPERTY INSTALL_RPATH "$ORIGIN/../lib")
endfunction()

option(STANDALONE_INSTALL "Create an 'install' for packaging which doesn't \
         require installation" off)
if (STANDALONE_INSTALL)
  message(STATUS "Setting an $ORIGIN-based RPATH on all executables")
  set_rpath_all_targets(${CMAKE_CURRENT_SOURCE_DIR})
endif()
