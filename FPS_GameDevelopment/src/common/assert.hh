#pragma once

#include <sstream>
#include <string>
#include "pretty_function.hh"

void assert_fail(const char* expr, const char* file, int line, const char* function, std::string const& msg);

#define gdassert(EXPR, MSG) ((EXPR) ? (void)0 : assert_fail(#EXPR, __FILE__, __LINE__, __PRETTY_FUNCTION__, static_cast<std::ostringstream&>(std::ostringstream() << MSG).str()))

#define gdfail(MSG) gdassert(false, MSG)
