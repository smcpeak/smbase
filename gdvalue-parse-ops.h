// gdvalue-parse-ops.h
// Dependency-laden operations for `gdvalue-parse.h`.

#ifndef GDVALUE_PARSE_OPS_H
#define GDVALUE_PARSE_OPS_H

#include "gdvalue-parse.h"             // decls for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-macros.h"          // {OPEN,CLOSE}_NAMESPACE


OPEN_NAMESPACE(gdv)


template <typename T>
T gdvOptTo(GDValue const &v)
{
  if (v.isNull()) {
    return T();
  }
  else {
    return gdvTo<T>(v);
  }
}


CLOSE_NAMESPACE(gdv)


#endif // GDVALUE_PARSE_OPS_H
