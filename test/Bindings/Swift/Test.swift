// RUN: SwiftHDL_Bindings_Swift_Test 2>&1 | FileCheck %s

import CxxStdlib
import SwiftHDL_Bindings_Support

let context = SwiftHDL.ScopedContext.create()

// CHECK: Hello, Swift!
print("Hello, Swift!")
