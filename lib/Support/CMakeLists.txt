add_swift_hdl_library(SwiftHDLBindingsSupport
    OpaquePointer.cpp
    ReferenceCountedPointer.cpp
    ScopedBuilder.cpp
    ScopedContext.cpp
    ScopedPassManager.cpp

    LINK_LIBS PRIVATE
    MLIRIR
    MLIRPass
    MLIRTransforms
    )
