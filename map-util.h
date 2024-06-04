// map-util.h
// Utilities related to std::map.

#ifndef SMBASE_MAP_UTIL_H
#define SMBASE_MAP_UTIL_H

// smbase
#include "xassert.h"                   // xassert

// libc++
#include <map>                         // std::map
#include <set>                         // set::set
#include <utility>                     // std::make_pair, std::move


// Return the set of keys in 'm'.
template <class K, class V>
std::set<K> keySet(std::map<K,V> const &m)
{
  std::set<K> ret;
  for (auto it = m.begin(); it != m.end(); ++it) {
    ret.insert((*it).first);
  }
  return ret;
}


// Return the set of values in 'm'.
template <class K, class V>
std::set<V> rangeSet(std::map<K,V> const &m)
{
  std::set<V> ret;
  for (auto it = m.begin(); it != m.end(); ++it) {
    ret.insert((*it).second);
  }
  return ret;
}


// Look up 'k' in 'm'.  If found, return its value.  Otherwise return
// 'V(0)', which for a pointer type is NULL.
template <class K, class V>
V atOrNull(std::map<K,V> const &m, K const &k)
{
  auto it = m.find(k);
  if (it != m.end()) {
    return (*it).second;
  }
  else {
    return V(0);
  }
}


// Insert '(k,v)' into 'm'.  Throw if 'k' is already mapped.
template <class K, class V>
void insertMapUnique(std::map<K,V> &map, K const &k, V const &v)
{
  auto it = map.insert(std::make_pair(k, v));
  xassert(it.second);
}


// Insert '(k,v)' into 'm', moving 'v'.  Throw if 'k' is already mapped.
template <class K, class V>
void insertMapUniqueMove(std::map<K,V> &map, K const &k, V &&v)
{
  auto it = map.insert(std::make_pair(k, std::move(v)));
  xassert(it.second);
}


#endif // SMBASE_MAP_UTIL_H
