// gdvalue-map.h
// Conversion between `GDValue` and `std::map`.

#ifndef SMBASE_GDVALUE_MAP_H
#define SMBASE_GDVALUE_MAP_H

#include "smbase/gdvalue-map-fwd.h"    // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parse.h"      // gdv::GDVTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <map>                         // std::map


OPEN_NAMESPACE(gdv)


template <typename K, typename V, typename C, typename A>
GDValue toGDValue(std::map<K,V,C,A> const &m)
{
  GDValue ret(GDVK_MAP);

  for (auto const &kv : m) {
    ret.mapSetValueAt(toGDValue(kv.first), toGDValue(kv.second));
  }

  return ret;
}


template <typename K, typename V, typename C, typename A>
struct GDVTo<std::map<K,V,C,A>> {
  static std::map<K,V,C,A> f(GDValue const &src)
  {
    checkIsMap(src);

    std::map<K,V,C,A> dest;

    for (auto const &kv : src.mapGet()) {
      GDValue const &k = kv.first;
      GDValue const &v = kv.second;

      dest.insert(std::make_pair(
        gdvTo<K>(k),
        gdvTo<V>(v)
      ));
    }

    return dest;
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_MAP_H
