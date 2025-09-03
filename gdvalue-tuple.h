// gdvalue-tuple.h
// Conversion between `GDValue` and `std::tuple`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_TUPLE_H
#define SMBASE_GDVALUE_TUPLE_H

#include "smbase/gdvalue-tuple-fwd.h"  // fwds for this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parser.h"     // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <cstddef>                     // std::size_t
#include <tuple>                       // std::{apply, tuple}
#include <utility>                     // std::index_sequence[_for]


OPEN_NAMESPACE(gdv)


template <typename... Elements>
GDValue toGDValue(std::tuple<Elements...> const &t)
{
  GDValue ret(GDVK_TUPLE);

  // Step 1: Define a function that, when passed any sequence of
  // references, will call `ret.tupleAppend()` on each, in order.
  auto lambda = [&ret](auto const &... elems)
  {
    /* Fold expression:

         (X(elems), ...)
          ^^^^^^^^
          pattern portion of the fold expression syntax.

       expands to:

         (X(e1), X(e2), ..., X(eN))

       The comma operator is used to sequence the calls, discarding
       their (void) return value.
    */
    (ret.tupleAppend(toGDValue(elems)), ...);
  };

  // Step 2: Apply that function to the elements in `t`.
  std::apply(lambda, t);

  return ret;
}


template <typename... Elements>
struct GDVPTo<std::tuple<Elements...>> {
  /* Helper that has access to the `Indices` as a pack that can be
     expanded simultaneously with `Elements`.

     We don't use the function parameter itself, we just need the
     template parameter, which is itself specified via deduction from
     the function argument.

     There does not seem to be any easy alternative that avoids having
     this extra parameter.  There is no syntax in C++ to make a pack
     from its constituents, so we need deduction somewhere to get a
     pack.  One way that avoids the extra parameter is class template
     partial specialization, but that gets fairly messy.  The optimizer
     should have no trouble removing the function parameter, so I'll
     tolerate it.
  */
  template <std::size_t... Indices>
  static std::tuple<Elements...> helper(
    GDValueParser const &p,
    std::index_sequence<Indices...> /*unused*/)
  {
    return std::tuple<Elements...>(
      // Simultaneously expand `Elements` and `Indices` to generate one
      // argument per element.  (The separating commas are implicit in
      // this syntax.)
      gdvpTo<Elements>(p.tupleGetValueAt(Indices)) ...
    );
  }

  // Primary conversion function.
  static std::tuple<Elements...> f(GDValueParser const &p)
  {
    // Make sure `p` refers to a tuple of the proper size.  But note
    // that this allows tagged tuples as well, ignoring the tag.  The
    // expectation is the caller will have already checked the tag, if
    // desired, before calling this function.
    p.checkTupleSize(sizeof...(Elements));

    return helper(p, std::index_sequence_for<Elements...>{});
  }
};


// Convenience function for parsing tuples.  This is more convenient
// when the tuple type is not already in hand.
template <typename... Elements>
std::tuple<Elements...> gdvpToTuple(GDValueParser const &p)
{
  return gdvpTo<std::tuple<Elements...>>(p);
}


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_TUPLE_H
