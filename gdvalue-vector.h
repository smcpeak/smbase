// gdvalue-vector.h
// Conversion between `GDValue` and `std::vector`.

#ifndef SMBASE_GDVALUE_VECTOR_H
#define SMBASE_GDVALUE_VECTOR_H

#include "smbase/gdvalue-vector-fwd.h" // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parser.h"     // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <vector>                      // std::vector


OPEN_NAMESPACE(gdv)


template <typename T, typename A>
GDValue toGDValue(std::vector<T,A> const &v)
{
  GDValue ret(GDVK_SEQUENCE);

  for (T const &t : v) {
    ret.sequenceAppend(toGDValue(t));
  }

  return ret;
}


template <typename T, typename A>
struct GDVPTo<std::vector<T,A>> {
  static std::vector<T,A> f(GDValueParser const &p)
  {
    p.checkIsSequence();

    std::vector<T,A> vec;

    // Iterate with an index so we can keep track of the path.
    for (GDVIndex i=0; i < p.containerSize(); ++i) {
      vec.push_back(gdvpTo<T>(p.sequenceGetValueAt(i)));
    }

    return vec;
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_VECTOR_H
