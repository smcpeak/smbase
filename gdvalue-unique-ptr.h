// gdvalue-unique-ptr.h
// Conversion between `GDValue` and `std::unique_ptr`.

#ifndef SMBASE_GDVALUE_UNIQUE_PTR_H
#define SMBASE_GDVALUE_UNIQUE_PTR_H

#include "gdvalue-unique-ptr-fwd.h"    // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parser.h"     // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <memory>                      // std::unique_ptr


OPEN_NAMESPACE(gdv)


template <typename T, typename D>
GDValue toGDValue(std::unique_ptr<T,D> const &p)
{
  return toGDValue(*p);
}


template <typename T, typename D>
struct GDVPTo<std::unique_ptr<T,D>> {
  // This simply wraps the result of `gdvpToNew` as a `unique_ptr`.
  static std::unique_ptr<T,D> f(GDValueParser const &p)
  {
    return std::unique_ptr<T,D>(gdvpToNew<T>(p));
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_UNIQUE_PTR_H
