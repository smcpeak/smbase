// gdvalue-list.h
// Conversion between `GDValue` and `std::list`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_LIST_H
#define SMBASE_GDVALUE_LIST_H

#include "smbase/gdvalue-list-fwd.h"   // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parse.h"      // gdv::GDVTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <list>                        // std::list


OPEN_NAMESPACE(gdv)


template <typename T, typename A>
GDValue toGDValue(std::list<T,A> const &v)
{
  GDValue ret(GDVK_SEQUENCE);

  for (T const &t : v) {
    ret.sequenceAppend(toGDValue(t));
  }

  return ret;
}


template <typename T, typename A>
struct GDVTo<std::list<T,A>> {
  static std::list<T,A> f(GDValue const &v)
  {
    checkIsSequence(v);

    std::list<T,A> vec;

    for (GDValue const &element : v.sequenceGet()) {
      vec.push_back(gdvTo<T>(element));
    }

    return vec;
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_LIST_H
