// ordered-map-ops.h
// Operations for `ordered-map` module.

// TODO: Apply header-analysis to fully populate this file.

#ifndef SMBASE_ORDERED_MAP_OPS_H
#define SMBASE_ORDERED_MAP_OPS_H

#include "smbase/ordered-map.h"        // interface for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/xassert.h"            // xassertPrecondition


OPEN_NAMESPACE(smbase)


template <typename KEY, typename VALUE>
OrderedMap<KEY, VALUE>::OrderedMap(
  std::initializer_list<value_type> ilist)
  : m_map(ilist),
    m_keyVector()
{
  // If this fails, there must have been a duplicate key.
  xassertPrecondition(m_map.size() == ilist.size());

  // The initial order is that of the initializer list.
  for (auto const &kv : ilist) {
    m_keyVector.push_back(kv.first);
  }

  selfCheck();
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_MAP_OPS_H
