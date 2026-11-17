// ordered-mru-flat-map-iface.h
// Interface decls for `ordered-mru-flat-map`.

// This file is in the public domain.

#ifndef SMBASE_ORDERED_MRU_FLAT_MAP_IFACE_H
#define SMBASE_ORDERED_MRU_FLAT_MAP_IFACE_H

#include "ordered-mru-flat-map-fwd.h"            // fwds for this module

#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue [f]
#include "smbase/ordered-flat-map-iface.h"       // OrderedFlatMap
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE

#include <type_traits>                           // std::{decay_t, enable_if_t, is_same_v}


OPEN_NAMESPACE(smbase)


/*
  OrderedFlatMap augmented with a one-element most-recently-used cache
  for lookups.

  This is intended for use within an in-process profiling infrastructure
  that I expect will repeatedly look up the same key in a given map.

  The public interface is the same as that of OrderedFlatMap, so it is
  not documented here.
*/
template <typename KEY,
          typename VALUE,
          typename COMPARE> // Defaults to std::less<KEY> in -fwd.h file.
class OrderedMRUFlatMap :
  // Privately inherit the underlying map so I call its protected
  // methods, but it is not exposed to clients.
  private OrderedFlatMap<KEY, VALUE, COMPARE> {

public:      // types
  using Base = OrderedFlatMap<KEY, VALUE, COMPARE>;
  using size_type = typename Base::size_type;

private:     // data
  // Most-recently-used lookup cache.  If the map is empty then this is
  // 0, otherwise it is always a valid array index, normally the most
  // recently looked up.
  //
  // Invariant: empty()?
  //   m_mruIndex==0 :
  //   (0 <= m_mruIndex < size())
  size_type m_mruIndex;

public:      // funcs
  inline ~OrderedMRUFlatMap();

  inline void selfCheck() const;

  // -------------------------- Constructors ---------------------------
  inline OrderedMRUFlatMap();

  inline OrderedMRUFlatMap(OrderedMRUFlatMap const &obj);
  inline OrderedMRUFlatMap(OrderedMRUFlatMap      &&obj);

  // ------------------------- Element access --------------------------
  inline bool contains(KEY const &key) const;
  inline VALUE const &valueAtKey(KEY const &key) const;
  inline KEY const &keyAtIndex(size_type index) const;
  inline VALUE const &valueAtIndex(size_type index) const;

  // ---------------------------- Capacity -----------------------------
  inline bool empty() const;
  inline size_type size() const;

  // ---------------------------- Modifiers ----------------------------
  template <typename K,
            typename V,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<K>, KEY>>
           >
  inline bool setValueAtKey(K &&key, V &&value);

  template <typename K,
            typename V,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<K>, KEY>>
           >
  inline void setValueAtNewKey(K &&key, V &&value);

  // Currently, the MRU cache only applies to this function.
  inline VALUE &valueRefAtKey(KEY const &key);

  inline void clear();

  // -------------------------- Serialization --------------------------
  operator gdv::GDValue() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_MRU_FLAT_MAP_IFACE_H
