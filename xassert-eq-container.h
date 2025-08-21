// xassert-eq-container.h
// `xassert` that two containers are equal, with diagnostics when unequal.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_XASSERT_EQ_CONTAINER_H
#define SMBASE_XASSERT_EQ_CONTAINER_H

#include "smbase/iter-and-end.h"                 // smbase::{compareIterAndEnds, constIterAndEnd}
#include "smbase/gdvalue.h"                      // gdv::toGDValue
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/stringb.h"                      // needed for xfailure_stringbc
#include "smbase/xassert.h"                      // xassert_stringbc

#include <set>                                   // std::set
#include <string_view>                           // std::string_view


OPEN_NAMESPACE(smbase)


// If the contents of `a` and `b`, assumed to be pointing into ordered
// containers, are equal, then do nothing.  Otherwise, throw `XAssert`
// with a diagnostic message about which one had which extra element.
// This uses `toGDValue` to stringify the elements.
//
// Although this is algorithmically just a lexicographic comparison, the
// "ordered" assumption affects how we interpret the result,
// specifically that it means the mismatched element is entirely absent
// from the other container.
//
template <typename COMPARATOR, typename ITER_AND_END>
void xassertEqualOrderedContainersIAE(
  COMPARATOR const &isLessThan,
  std::string_view aLabel,
  ITER_AND_END a,
  std::string_view bLabel,
  ITER_AND_END b)
{
  using gdv::toGDValue;

  int cmp = compareLexicographicallyIAE(
    isLessThan, a /*INOUT*/, b /*INOUT*/);

  if (cmp < 0) {
    xfailure_stringbc(
      "Expected equal sets, but " << aLabel <<
      " has element " << toGDValue(*a) <<
      " that " << bLabel << " lacks.");
  }
  else if (cmp > 0) {
    xfailure_stringbc(
      "Expected equal sets, but " << bLabel <<
      " has element " << toGDValue(*b) <<
      " that " << aLabel << " lacks.");
  }
}


// Same, but specifically for sets.
template <typename K, typename C, typename A>
void xassertEqualSets(
  std::string_view aLabel,
  std::set<K,C,A> const &a,
  std::string_view bLabel,
  std::set<K,C,A> const &b)
{
  // Comparator to use.
  C isLessThan;

  xassertEqualOrderedContainersIAE(
    isLessThan,
    aLabel, constIterAndEnd(a),
    bLabel, constIterAndEnd(b));
}


// Supply the labels from the source code names.
#define XASSERT_EQUAL_SETS(a, b) \
  xassertEqualSets(#a, a, #b, b)


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_XASSERT_EQ_CONTAINER_H
