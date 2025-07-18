// astlist-gdvalue.h
// Functions involving `ASTList` and `GDValue`.

// See license.txt for copyright and terms of use.

// This header is separate from `astlist.h` in order to avoid loading
// that down with a somewhat heavy dependency that often is not needed.

#ifndef SMBASE_ASTLIST_GDVALUE_H
#define SMBASE_ASTLIST_GDVALUE_H

#include "smbase/astlist.h"            // ASTList
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvalue-parser.h"     // gdv::GDValueParser


// Convert `lst` to a GDV sequence.
template <typename T>
gdv::GDValue toGDValue(ASTList<T> const &lst)
{
  using namespace gdv;

  GDValue s(GDVK_SEQUENCE);

  FOREACH_ASTLIST(T, lst, iter) {
    s.sequenceAppend(toGDValue(*(iter.data())));
  }

  return s;
}


template <typename T>
struct gdv::GDVPTo<ASTList<T> > {
  static ASTList<T> f(GDValueParser const &p)
  {
    p.checkIsSequence();

    ASTList<T> ret;

    // Iterate with an index so we can keep track of the path.
    for (GDVIndex i=0; i < p.containerSize(); ++i) {
      ret.append(gdv::gdvpToNew<T>(p.sequenceGetValueAt(i)));
    }

    return ret;
  }
};


#endif // SMBASE_ASTLIST_GDVALUE_H
