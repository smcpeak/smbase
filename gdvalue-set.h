// gdvalue-set.h
// Conversion between `GDValue` and `std::set`.

#ifndef SMBASE_GDVALUE_SET_H
#define SMBASE_GDVALUE_SET_H

#include "smbase/gdvalue-set-fwd.h"    // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parse.h"      // gdv::GDVTo
#include "smbase/gdvalue-parser.h"     // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <set>                         // std::set


OPEN_NAMESPACE(gdv)


template <typename V, typename C, typename A>
GDValue toGDValue(std::set<V,C,A> const &s)
{
  GDValue ret(GDVK_SET);

  for (auto const &v : s) {
    ret.setInsert(toGDValue(v));
  }

  return ret;
}


template <typename V, typename C, typename A>
struct GDVTo<std::set<V,C,A>> {
  static std::set<V,C,A> f(GDValue const &src)
  {
    checkIsSet(src);

    std::set<V,C,A> dest;

    for (GDValue const &v : src.setGet()) {
      dest.insert(gdvTo<V>(v));
    }

    return dest;
  }
};


template <typename V, typename C, typename A>
struct GDVPTo<std::set<V,C,A>> {
  static std::set<V,C,A> f(GDValueParser const &p)
  {
    p.checkIsSet();

    std::set<V,C,A> dest;

    for (GDValue const &v : p.setGet()) {
      dest.insert(gdvpTo<V>(p.setGetValue(v)));
    }

    return dest;
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_SET_H
