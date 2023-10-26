// RUN: SwiftHDL_Bindings_Swift_Test 2>&1 | FileCheck %s

import CxxStdlib
import SwiftHDL_Bindings_Support

var context = SwiftHDL.ScopedContext.create()

let x = context.__getContextUnsafe();

// CHECK: Hello, Swift!
print("Hello, Swift!")
