// map-utils.h
// Utilities related to std::map.

#ifndef SMBASE_MAP_UTILS_H
#define SMBASE_MAP_UTILS_H

#include <map>                         // std::map
#include <set>                         // set::set


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


#endif // SMBASE_MAP_UTILS_H
