// gdvalue-unique-ptr.h
// Conversion between `GDValue` and `std::unique_ptr`.

#ifndef SMBASE_GDVALUE_UNIQUE_PTR_H
#define SMBASE_GDVALUE_UNIQUE_PTR_H

#include "gdvalue-unique-ptr-fwd.h"    // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parse.h"      // gdv::GDVTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <memory>                      // std::unique_ptr


OPEN_NAMESPACE(gdv)


template <typename T, typename D>
GDValue toGDValue(std::unique_ptr<T,D> const &p)
{
  return toGDValue(*p);
}


template <typename T, typename D>
struct GDVTo<std::unique_ptr<T,D>> {
  // This simply wraps the result of `gdvToNew` as a `unique_ptr`.
  static std::unique_ptr<T,D> f(GDValue const &v)
  {
    return std::unique_ptr<T,D>(gdvToNew<T>(v));
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_UNIQUE_PTR_H
