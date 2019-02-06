#pragma once

/**
 * Prevents inlining for a given function.
 *
 * Example:
 *
 * NOINLINE void doNotInlineMe() { return; }
 */

#ifndef COMPILER_MSVC
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE __declspec(noinline)
#endif
