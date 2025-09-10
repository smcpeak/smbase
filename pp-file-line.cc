// pp-file-line.cc
// Code for `pp-file-line` module.

#include "pp-file-line.h"              // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP
#include "smbase/string-util.h"        // withoutDirectoryPrefix

#include <iostream>                    // std::ostream


OPEN_NAMESPACE(smbase)


char const *PreprocFileLine::fileWithoutDir() const
{
  return withoutDirectoryPrefix(file());
}


void PreprocFileLine::write(std::ostream &os) const
{
  os << fileWithoutDir() << ':' << line();
}


CLOSE_NAMESPACE(smbase)


// EOF
