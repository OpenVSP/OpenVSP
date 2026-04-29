//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <string>
using std::string;

// Returns a multi-line string containing the current call stack, with
// demangled symbol names and, when compiled with debug information,
// source file names and line numbers.
//
// skip: number of additional frames to omit from the top of the trace
//       (GetStackTrace itself is always omitted automatically).
//
// Note: not async-signal-safe; do not call from a signal handler.
string GetStackTrace( int skip = 0 );

// Writes the stack trace to stderr, wrapped in a header/footer banner.
void PrintStackTrace( int skip = 0 );

#endif
