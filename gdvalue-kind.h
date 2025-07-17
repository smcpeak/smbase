// gdvalue-kind.h
// `GDValueKind` enumeration.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_KIND_H
#define SMBASE_GDVALUE_KIND_H

#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


/* Possible kinds of GDValues.

   The order of the enumerators is also the order into which the kinds
   sort, *except* that Integer and SmallInteger sort with respect to
   each other according to their numerical value, regardless of the
   classification as "small" or not.  That is, we have:

     large neg < small neg < 0 < small pos < large pos

   (Zero is actually a small non-negative integer.)
*/
enum GDValueKind : unsigned char {
  // ---- Scalars ----
  // Scalars are tree "leaves" in that they do not contain other
  // GDValues.

  // Symbol: An identifier-like string that acts as a name of something
  // defined elsewhere.  This includes the special symbols `null`,
  // `false`, and `true`.
  GDVK_SYMBOL,

  // Integer: Unbounded mathematical integer.
  GDVK_INTEGER,

  // Small integer: A logical subclass of Integer that fits into the
  // `GDVSmallInteger` type.
  GDVK_SMALL_INTEGER,

  // String: Sequence of Unicode characters encoded as UTF-8.
  GDVK_STRING,

  // ---- Containers ----
  // Containers are potential interior tree "nodes" in that they can
  // contain other GDValues.

  // Sequence: Ordered sequence of values.
  GDVK_SEQUENCE,

  // Tagged sequence: A symbol and a sequence.
  GDVK_TAGGED_SEQUENCE,

  // Tuple: Another kind of sequence, at least from a representation
  // perspective.  (See gdvalue-design.txt, "Tuples versus sequences".)
  GDVK_TUPLE,
  GDVK_TAGGED_TUPLE,

  // Set: Unordered set of (unique) values.
  GDVK_SET,

  // Tagged set: A symbol and a set.
  GDVK_TAGGED_SET,

  // Map: Set of (key, value) pairs that are indexed by key.
  GDVK_MAP,

  // Tagged map: A symbol and a map.
  GDVK_TAGGED_MAP,

  // Ordered Map: A map where the entries have an externally-imposed
  // order, typically the insertion order.
  GDVK_ORDERED_MAP,

  // Tagged ordered map: A symbol and an ordered map.
  GDVK_TAGGED_ORDERED_MAP,

  NUM_GDVALUE_KINDS
};


// The following functions are defined in `gdvalue.cc`.

// Return a string like "GDVK_SYMBOL", or "GDVK_invalid" if 'gdvk' is
// invalid.
char const *toString(GDValueKind gdvk);

// Return a string like "symbol" that is how the data type would be
// described in prose.
char const *kindCommonName(GDValueKind gdvk);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_KIND_H
