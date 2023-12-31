##===- CMakeLists.txt - ESI runtime CMake ---------------------*- cmake -*-===//
##
## Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
## See https://llvm.org/LICENSE.txt for license information.
## SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
##
##===----------------------------------------------------------------------===//
##
## Compile definitions for the ESI runtime. Distributed with an ESI compiler as
## part of the ESI collateral. For now, we require that users compile this
## themselves since it needs to be compiled for each Python version and OS then
## packed together. Eventually, we'll just be distributing (lots of) binaries.
##
## We require Python development package and pybind11 to compile the Python API.
##
## ESI cosimulation requires Cap'nProto as we use it for our RPC with the
## simulator. It must be fetched separately, but is optional if you don't want
## cosimulation.
##
## DO NOT EDIT!
## This file is distributed as part of an ESI package. The source for this file
## should always be modified within CIRCT.
##
##===----------------------------------------------------------------------===//

project(ESIRuntime)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(ZLIB REQUIRED)

include_directories(cpp/include)

set(ESIRuntimeSources
  cpp/lib/Accelerator.cpp
  cpp/lib/StdServices.cpp
)
set(ESIRuntimeLinkLibraries
  ZLIB::ZLIB
)
set(ESIPythonRuntimeSources
  python/esi/__init__.py
  python/esi/accelerator.py
)

# If Cap'nProto hasn't been explicitly disabled, find it.
option(CAPNP_DISABLE "Disable Cap'nProto (needed for cosimulation).")
if (CAPNP_DISABLE)
  message (STATUS "Disabling Cap'nProto (needed for cosimulation).")
elseif(NOT CapnProto_FOUND) # Check if someone else already imported.
  if(DEFINED CAPNP_PATH)
    set(ENV{PKG_CONFIG_PATH}
      "${CAPNP_PATH}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    find_package(CapnProto CONFIG PATHS ${CAPNP_PATH})
  else()
    set(ENV{PKG_CONFIG_PATH}
      "${CMAKE_CURRENT_SOURCE_DIR}/ext/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    find_package(CapnProto CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ext")
  endif()
endif()

# If Cap'nProto has been found, generate the headers and definitions.
if(CapnProto_FOUND)
  set(ESI_COSIM true)
  message("-- ESI cosim enabled")

  message(STATUS "Found Cap'nProto at ${CapnProto_DIR}.")
  set(CMAKE_INSTALL_RPATH ${capnp_LIBDIR})
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  add_subdirectory(cosim)

  set(ESIRuntimeSources
    ${ESIRuntimeSources}
    cpp/lib/backends/Cosim.cpp
  )
  set(ESIRuntimeLinkLibraries
    ${ESIRuntimeLinkLibraries}
    EsiCosimCapnp
  )
endif()


# The core API. For now, compile the backends into it directly.
# TODO: make this a plugin architecture.
add_library(ESIRuntime SHARED
  ${ESIRuntimeSources}
)
target_link_libraries(ESIRuntime PRIVATE ${ESIRuntimeLinkLibraries})

# The esiquery tool is a simple wrapper around the SysInfo API.
add_executable(esiquery
  cpp/tools/esiquery.cpp
)
target_link_libraries(esiquery PRIVATE ESIRuntime)

# Global variable for the path to the ESI runtime for use by tests.
set(ESIRuntimePath "${CMAKE_CURRENT_BINARY_DIR}"
  CACHE INTERNAL "Path to ESI runtime" FORCE)

# Pybind11 is used to wrap the ESIRuntime APIs.
find_package(Python COMPONENTS Interpreter Development)
if(Python_FOUND)
  if(pybind11_DIR)
    message(STATUS "Using explicit pybind11 cmake directory: ${pybind11_DIR} (-Dpybind11_DIR to change)")
  else()
    message(STATUS "Checking for pybind11 in python path...")
    execute_process(
      COMMAND "${Python_EXECUTABLE}"
      -c "import pybind11;print(pybind11.get_cmake_dir(), end='')"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE STATUS
      OUTPUT_VARIABLE PACKAGE_DIR
      ERROR_QUIET)
    if(NOT STATUS EQUAL "0")
      message(FATAL_ERROR "pybind11 not found (install via 'pip install pybind11' or set pybind11_DIR)")
    endif()
    message(STATUS "found (${PACKAGE_DIR})")
    set(pybind11_DIR "${PACKAGE_DIR}")
  endif()

  # Now, find pybind11.
  find_package(pybind11 CONFIG)
  if (NOT pybind11_FOUND)
    message (STATUS "Could not find pybind11. Disabling Python API.")
  else()
    # Compile Pybind11 module and copy to the correct python directory.
    pybind11_add_module(esiCppAccel python/esi/esiCppAccel.cpp)
    target_link_libraries(esiCppAccel PRIVATE ESIRuntime)
    add_custom_command(
        TARGET esiCppAccel
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:esiCppAccel>
            "${CMAKE_CURRENT_BINARY_DIR}/python/esi/$<TARGET_FILE_NAME:esiCppAccel>"
    )

    # Copy each of the Python sources to the build dir.
    foreach(pysrc ${ESIPythonRuntimeSources})
      add_custom_command(
          OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${pysrc}
          COMMAND ${CMAKE_COMMAND} -E copy
              ${CMAKE_CURRENT_SOURCE_DIR}/${pysrc}
              ${CMAKE_CURRENT_BINARY_DIR}/${pysrc}
          DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${pysrc}
      )
    endforeach()

    # Custom target for the Python runtime just aggregates the python sources
    # and Pybind11 module.
    add_custom_target(ESIPythonRuntime
      DEPENDS
        ${ESIPythonRuntimeSources}
        esiCppAccel
    )

  endif()
endif()
