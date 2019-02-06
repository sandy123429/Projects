#pragma once

#include "no_inline.hh"

/**
 * This header defines the `debug::breakpoint()' function
 * Calling this function will trigger a breakpoint if debugger is attached
 */

namespace debug
{
NOINLINE void breakpoint();
}
