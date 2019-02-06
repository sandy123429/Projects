#include "breakpoint.hh"

#ifdef COMPILER_MSVC
#include <intrin.h>
#endif

// TODO(pt): Maybe use IsDebuggerPresent (http://msdn.microsoft.com/en-us/library/ms680345%28VS.85%29.aspx)

void debug::breakpoint()
{
#ifdef COMPILER_MSVC
    __debugbreak();
#else
    __builtin_trap();
#endif
}
