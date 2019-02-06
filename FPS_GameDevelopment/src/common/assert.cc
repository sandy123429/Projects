#include "assert.hh"

#include <iostream>

#include "breakpoint.hh"

void assert_fail(const char *expr, const char *file, int line, const char *function, const std::string &msg)
{
    std::cerr << "Assertion `" << expr << "' failed." << std::endl;
    std::cerr << "  File: " << file << ":" << line << std::endl;
    std::cerr << "  Func: " << function << std::endl;
    std::cerr << "  Msg:  " << msg << std::endl;

    debug::breakpoint();
}
