include_guard()

function(add_swift_hdl_dialect dialect dialect_namespace)
  add_mlir_dialect(${ARGV})
  add_dependencies(swift-hdl-headers MLIR${dialect}IncGen)
endfunction()

function(add_swift_hdl_interface interface)
  add_mlir_interface(${ARGV})
  add_dependencies(swift-hdl-headers MLIR${interface}IncGen)
endfunction()

# Additional parameters are forwarded to tablegen.
function(add_swift_hdl_doc tablegen_file output_path command)
  set(LLVM_TARGET_DEFINITIONS ${tablegen_file}.td)
  string(MAKE_C_IDENTIFIER ${output_path} output_id)
  tablegen(MLIR ${output_id}.md ${command} ${ARGN})
  set(GEN_DOC_FILE ${SWIFT_HDL_BINARY_DIR}/docs/${output_path}.md)
  add_custom_command(
          OUTPUT ${GEN_DOC_FILE}
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_BINARY_DIR}/${output_id}.md
                  ${GEN_DOC_FILE}
          DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${output_id}.md)
  add_custom_target(${output_id}DocGen DEPENDS ${GEN_DOC_FILE})
  add_dependencies(swift-hdl-doc ${output_id}DocGen)
endfunction()

function(add_swift_hdl_dialect_doc dialect dialect_namespace)
  add_swift_hdl_doc(
    ${dialect} Dialects/${dialect}
    -gen-dialect-doc -dialect ${dialect_namespace})
endfunction()

function(add_swift_hdl_library name)
  add_mlir_library(${ARGV})
  add_swift_hdl_library_install(${name})
endfunction()

macro(add_swift_hdl_executable name)
  add_llvm_executable(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "SwiftHDL executables")
endmacro()

macro(add_swift_hdl_tool name)
  if (NOT SWIFT_HDL_BUILD_TOOLS)
    set(EXCLUDE_FROM_ALL ON)
  endif()

  add_swift_hdl_executable(${name} ${ARGN})

  if (SWIFT_HDL_BUILD_TOOLS)
    get_target_export_arg(${name} SwiftHDL export_to_swifthdltargets)
    install(TARGETS ${name}
      ${export_to_swifthdltargets}
      RUNTIME DESTINATION "${SWIFT_HDL_TOOLS_INSTALL_DIR}"
      COMPONENT ${name})

    if(NOT CMAKE_CONFIGURATION_TYPES)
      add_llvm_install_targets(install-${name}
        DEPENDS ${name}
        COMPONENT ${name})
    endif()
    set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_EXPORTS ${name})
  endif()
endmacro()

# Adds a SwiftHDL library target for installation.  This should normally only be
# called from add_swift_hdl_library().
function(add_swift_hdl_library_install name)
  install(TARGETS ${name} COMPONENT ${name} EXPORT SwiftHDLTargets)
  set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_ALL_LIBS ${name})
  set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_EXPORTS ${name})
endfunction()

function(add_swift_hdl_dialect_library name)
  set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_DIALECT_LIBS ${name})
  add_swift_hdl_library(${ARGV} DEPENDS swift-hdl-headers)
endfunction()

function(add_swift_hdl_conversion_library name)
  set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_CONVERSION_LIBS ${name})
  add_swift_hdl_library(${ARGV} DEPENDS swift-hdl-headers)
endfunction()

function(add_swift_hdl_translation_library name)
  set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_TRANSLATION_LIBS ${name})
  add_swift_hdl_library(${ARGV} DEPENDS swift-hdl-headers)
endfunction()

function(add_swift_hdl_verification_library name)
  set_property(GLOBAL APPEND PROPERTY SWIFT_HDL_VERIFICATION_LIBS ${name})
  add_swift_hdl_library(${ARGV} DEPENDS swift-hdl-headers)
endfunction()
