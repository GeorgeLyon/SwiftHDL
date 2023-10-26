add_swift_hdl_library(SwiftFIRRTL
  Dialect.cpp

  LINK_LIBS
    PUBLIC
      CIRCTFIRRTL
      CIRCTFirtool
    PRIVATE
      SwiftHDLBindingsSupport
  )
