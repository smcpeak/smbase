// gdvn.cc
// Program to read and write GDVN.

// This file is in the public domain.

#include "exc.h"                       // smbase::XBase
#include "gdvalue.h"                   // gdv::GDValue

#include <iostream>                    // std::{cin, cout, cerr, endl}

using namespace gdv;
using namespace smbase;


int main(int argc, char **argv)
{
  // File to read, or nullptr for stdin.
  char const *fname = nullptr;

  if (argc >= 2) {
    fname = argv[1];
  }

  try {
    GDValue value;
    if (fname) {
      value = GDValue::readFromFile(fname);
    }
    else {
      value = GDValue::readFromStream(std::cin);
    }

    value.writeLines(std::cout);
  }
  catch (XBase &x) {
    std::cerr << x.why() << std::endl;
    return 2;
  }

  return 0;
}


// EOF
