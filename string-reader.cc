// string-reader.cc
// Code for `string-reader.h`.

#include "smbase/string-reader.h"      // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <sstream>                     // std::istringstream


OPEN_NAMESPACE(smbase)


StringReader::~StringReader()
{}


StringReader::StringReader(
  std::string const &str,
  std::optional<std::string> fileName)
  : DataWrapper<std::istringstream>(str),
    Reader(istrstr(), fileName)
{}


CLOSE_NAMESPACE(smbase)


// EOF
