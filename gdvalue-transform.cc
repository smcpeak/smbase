// gdvalue-transform.cc
// Code for `gdvalue-transform` module.

#include "gdvalue-transform.h"         // this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvsymbol.h"          // gdv::GDVSymbol
#include "smbase/ordered-map.h"        // smbase::OrderedMap
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


GDValue GDValueTransform::transform(GDValue const &v)
{
  if (!v.isContainer()) {
    return v;
  }

  GDValue out(v.getKind());

  if (v.isSequence()) {
    for (GDValue const &elt : v.sequenceIterableC()) {
      out.sequenceAppend(transform(elt));
    }
  }

  else if (v.isTuple()) {
    for (GDValue const &elt : v.tupleIterableC()) {
      out.tupleAppend(transform(elt));
    }
  }

  else if (v.isSet()) {
    for (GDValue const &elt : v.setIterableC()) {
      // Discard duplicates.
      out.setInsert(transform(elt));
    }
  }

  else if (v.isMap()) {
    for (auto const &kv : v.mapIterableC()) {
      // Discard entries with colliding keys.
      out.mapInsertValueAt(
        transform(kv.first),
        transform(kv.second));
    }
  }

  else if (v.isOrderedMap()) {
    for (auto const &kv : v.orderedMapIterableC()) {
      // Discard entries with colliding keys.
      out.orderedMapInsertValueAt(
        transform(kv.first),
        transform(kv.second));
    }
  }

  else {
    xfailure_stringbc(
      "Unhandled container kind: " << v.getKindCommonName());
  }

  if (out.isTaggedContainer()) {
    out.taggedContainerSetTag(
      transformContainerTag(v.taggedContainerGetTag()));
  }

  return out;
}


GDVSymbol GDValueTransform::transformContainerTag(GDVSymbol tag)
{
  return tag;
}


GDValue deepCopyGDValue(GDValue const &v)
{
  GDValueTransform transform;
  return transform.transform(v);
}


CLOSE_NAMESPACE(gdv)


// EOF
