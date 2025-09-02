// sm-span-util.cc
// Code for `sm-span-util` module.

#include "sm-span-util.h"              // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-span.h"            // smbase::Span

#include <sstream>                     // std::ostringstream
#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


std::string joinTerminate(Span<std::string const> span,
                          std::string const &sep)
{
  std::ostringstream oss;
  for (std::string const &s : span) {
    oss << s << sep;
  }
  return oss.str();
}


CLOSE_NAMESPACE(smbase)


// EOF
