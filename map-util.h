// map-util.h
// Utilities related to `std::map`.

// This file is in the public domain.

// TODO: All function names should start with "map".

// TODO: Put all functions into `smbase` namespace.

#ifndef SMBASE_MAP_UTIL_H
#define SMBASE_MAP_UTIL_H

// smbase
#include "sm-macros.h"                 // DEPRECATED
#include "xassert.h"                   // xassert

// libc++
#include <iostream>                    // std::ostream
#include <map>                         // std::map
#include <optional>                    // std::optional
#include <set>                         // set::set
#include <utility>                     // std::make_pair, std::move


// ----------------------------- Map->Set ------------------------------
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


// Insert all keys from 'src', presumably a map, into 'dest', presumably
// a set.
//
// TODO: I think I should remove this in favor of `keySet`.  Or, change
// `keySet` to call this function.
template <class DestSet, class SrcMap>
void mapInsertAllKeys(DestSet &dest, SrcMap const &src)
{
  for (auto const &kv : src) {
    dest.insert(kv.first);
  }
}


// ------------------------------ Lookup -------------------------------
// Look up 'k' in 'm'.  If found, return its value.  Otherwise return
// 'V(0)', which for a pointer type is NULL.
template <class K, class V>
V mapFindOrNull(std::map<K,V> const &m, K const &k)
{
  auto it = m.find(k);
  if (it != m.end()) {
    return (*it).second;
  }
  else {
    return V(0);
  }
}


template <class K, class V>
V atOrNull(std::map<K,V> const &m, K const &k)
  DEPRECATED("Use `mapFindOrNull` instead.");

template <class K, class V>
V atOrNull(std::map<K,V> const &m, K const &k)
{
  return mapFindOrNull(m, k);
}


// Return an optional iterator from a map lookup.
template <class K, class V>
std::optional<typename std::map<K,V>::const_iterator> mapFindOpt(
  std::map<K,V> const &m,
  K const &k)
{
  auto it = m.find(k);
  if (it != m.end()) {
    return std::make_optional(it);
  }
  else {
    return std::optional<typename std::map<K,V>::const_iterator>();
  }
}


// ---------------------------- Insertion ------------------------------
// I don't like to have to say 'make_pair' all the time, and I'm
// ambivalent about using the initializer list syntax, so this function
// encapsulates insertion.
//
// Returns true if the item was inserted (otherwise, it was already in
// the set).
template <class K, class V>
bool mapInsert(std::map<K,V> &m, K const &k, V const &v)
{
  auto res = m.insert(std::make_pair(k, v));
  return res.second;
}


// Insert '(k,v)' into 'm'.  Throw if 'k' is already mapped.
template <class K, class V>
void mapInsertUnique(std::map<K,V> &map, K const &k, V const &v)
{
  auto it = map.insert(std::make_pair(k, v));
  xassert(it.second);
}


// Insert '(k,v)' into 'm', moving 'v'.  Throw if 'k' is already mapped.
template <class K, class V>
void mapInsertUniqueMove(std::map<K,V> &map, K const &k, V &&v)
{
  auto it = map.insert(std::make_pair(k, std::move(v)));
  xassert(it.second);
}


// ----------------------------- Removal -------------------------------
// Remove the mapping for `k` if one exists.  Return true if it existed.
template <class K, class V>
bool mapRemove(std::map<K,V> &map, K const &k)
{
  return map.erase(k) > 0;
}


// Remove the mapping for `k`, which must exist.
template <class K, class V>
void mapRemoveExisting(std::map<K,V> &map, K const &k)
{
  bool erased = mapRemove(map, k);
  xassert(erased);
}


// Remove the mapping for `k`, which must exist, and return the value at
// that location by moving it out of the map.
template <class K, class V>
V mapMoveValueAt(std::map<K,V> &map, K const &k)
{
  auto it = map.find(k);
  xassert(it != map.end());

  // Move the value out of the node.
  V ret = std::move((*it).second);

  // Remove the node along with its now-indeterminate value.
  map.erase(it);

  return ret;
}


// ----------------------------- Map->Map ------------------------------
template <class K, class V>
void mapInsertAll(std::map<K,V> &dest, std::map<K,V> const &src)
{
  for (auto const &kv : src) {
    dest.insert(kv);
  }
}


// Compute and return a map from value to key.  This asserts that the
// values are unique.
template <class K, class V>
std::map<V,K> mapInvert(std::map<K,V> const &src)
{
  std::map<V,K> dest;
  for (auto const &kv : src) {
    mapInsertUnique(dest, kv.second, kv.first);
  }
  return dest;
}


// -------------------------- Printing maps ----------------------------
template <class K, class V, class PRINT_KEY, class PRINT_VALUE>
void mapWrite(
  std::ostream &os,
  std::map<K,V> const &m,
  PRINT_KEY printKey,
  PRINT_VALUE printValue)
{
  os << "{";

  int ct = 0;
  for (auto const &kv : m) {
    if (ct > 0) {
      os << ",";
    }
    os << " ";
    printKey(os, kv.first);
    os << ": ";
    printValue(os, kv.second);
    ++ct;
  }

  if (ct > 0) {
    os << " ";
  }
  os << "}";
}


// Like above, but writing an array of pairs instead of a JSON map,
// since JSON maps can only have strings as keys.
template <class K, class V, class PRINT_KEY, class PRINT_VALUE>
void mapWriteAsArray(
  std::ostream &os,
  std::map<K,V> const &m,
  PRINT_KEY printKey,
  PRINT_VALUE printValue)
{
  os << "[";

  int ct = 0;
  for (auto const &kv : m) {
    if (ct > 0) {
      os << ",";
    }
    os << " [";
    printKey(os, kv.first);
    os << ", ";
    printValue(os, kv.second);
    os << "]";
    ++ct;
  }

  if (ct > 0) {
    os << " ";
  }
  os << "]";
}


template <class K, class V>
std::ostream& operator<< (std::ostream &os, std::map<K,V> const &m)
{
  mapWrite(
    os,
    m,
    [](std::ostream &os, K const &k) -> void {
      os << k;
    },
    [](std::ostream &os, V const &v) -> void {
      os << v;
    });

  return os;
}


#endif // SMBASE_MAP_UTIL_H
