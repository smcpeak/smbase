// gdvalue-unique-ptr.h
// Conversion between `GDValue` and `std::unique_ptr`.

#ifndef SMBASE_GDVALUE_UNIQUE_PTR_H
#define SMBASE_GDVALUE_UNIQUE_PTR_H

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parse.h"      // gdv::GDVTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <memory>                      // std::unique_ptr


OPEN_NAMESPACE(gdv)


template <typename T>
GDValue toGDValue(std::unique_ptr<T> const &p)
{
  return toGDValue(*p);
}


template <typename T>
struct GDVTo<std::unique_ptr<T>> {
  // This simply wraps the result of `gdvToNew` as a `unique_ptr`.
  static std::unique_ptr<T> f(GDValue const &v)
  {
    return std::unique_ptr<T>(gdvToNew<T>(v));
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_UNIQUE_PTR_H
