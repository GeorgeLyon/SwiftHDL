// RUN: SwiftHDL_Bindings_Swift_Test 2>&1 | FileCheck %s

import CxxStdlib
import SwiftHDL

let context = SwiftHDL.ScopedContext.create()
circt.firrtl.loadFIRRTLDialect(context.__getContextUnsafe());

// CHECK: Hello, Swift!
print("Hello, Swift!")
