// gdvalue-parser-ops.h
// Operations for `gdvalue-parser.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_PARSER_OPS_H
#define SMBASE_GDVALUE_PARSER_OPS_H

#include "smbase/gdvalue-parser.h"     // decls for this module

#include "smbase/sm-macros.h"          // {OPEN,CLOSE}_NAMESPACE

#include <optional>                    // std::optional


OPEN_NAMESPACE(gdv)


template <typename T>
T gdvpOptTo(std::optional<GDValueParser> const &p)
{
  if (!p.has_value()) {
    return T();
  }
  else {
    return gdvpTo<T>(p.value());
  }
}


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_PARSER_OPS_H
