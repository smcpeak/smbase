// gdvalue-either.h
// `GDValue` de/serialization for `Either`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_EITHER_H
#define SMBASE_GDVALUE_EITHER_H

#include "smbase/gdvalue-either-fwd.h" // fwds for this module

#include "smbase/either.h"             // smbase::Either
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parser.h"     // gdv::GDVPTo
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/stringb.h"            // stringb


OPEN_NAMESPACE(gdv)


template <typename LEFT, typename RIGHT>
gdv::GDValue toGDValue(smbase::Either<LEFT, RIGHT> const &e)
{
  // Use a tagged one-element tuple.  (For some pairs of types, the
  // individual values are sufficiently distinct, but it would take a
  // lot of complexity to detect when that is and exploit it.)
  GDValue ret(GDVK_TAGGED_TUPLE);

  if (e.isLeft()) {
    ret.taggedContainerSetTag("left"_sym);
    ret.tupleSetValueAt(0, toGDValue(e.leftC()));
  }
  else {
    ret.taggedContainerSetTag("right"_sym);
    ret.tupleSetValueAt(0, toGDValue(e.rightC()));
  }

  return ret;
}


template <typename LEFT, typename RIGHT>
struct GDVPTo<smbase::Either<LEFT, RIGHT>> {
  static smbase::Either<LEFT, RIGHT> f(GDValueParser const &p)
  {
    p.checkTupleSize(1);
    p.checkIsTaggedContainer();

    if (p.taggedContainerGetTagName() == "left") {
      return gdvpTo<LEFT>(p.tupleGetValueAt(0));
    }
    else {
      if (p.taggedContainerGetTagName() != "right") {
        p.throwError(stringb(
          "Expected tuple tag `left` or `right`, not " <<
          p.taggedContainerGetTag() << "."));
      }
      return gdvpTo<RIGHT>(p.tupleGetValueAt(0));
    }
  }
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_EITHER_H
