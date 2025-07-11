// gdvalue-json.cc
// Code for `gdvalue-json.h`.

#include "smbase/gdvalue-json.h"       // this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/ordered-map-ops.h"    // smbase::OrderedMap method definitions
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, NULLABLE

#include <optional>                    // std::optional
#include <string_view>                 // std::string_view


OPEN_NAMESPACE(gdv)


// ---------------------- Convert GDValue to JSON ----------------------
// Given a sequence of GDValues, return a sequence of JSO values.
static GDValue gdvSequenceToJSOSequence(GDValue const &gdvSeq)
{
  GDValue ret(GDVK_SEQUENCE);

  for (GDValue const &gdvElt : gdvSeq.sequenceIterableC()) {
    ret.sequenceAppend(gdvToJSO(gdvElt));
  }

  return ret;
}


// Given a tuple, return a JSO sequence of its elements.
static GDValue gdvTupleToJSOSequence(GDValue const &tuple)
{
  GDValue ret(GDVK_SEQUENCE);

  for (GDValue const &element : tuple.tupleIterableC()) {
    ret.sequenceAppend(gdvToJSO(element));
  }

  return ret;
}


// Given a set, return a sequence of its JSO elements in the intrinsic
// order of the original GDValues.
static GDValue gdvSetToJSOSequence(GDValue const &src)
{
  GDValue ret(GDVK_SEQUENCE);

  for (GDValue const &element : src.setIterableC()) {
    ret.sequenceAppend(gdvToJSO(element));
  }

  return ret;
}


// True if every key in `m` is a string.
static bool allMapKeysAreStrings(GDValue const &m)
{
  for (GDVMapEntry const &kv : m.mapIterableC()) {
    if (!kv.first.isString()) {
      return false;
    }
  }
  return true;
}


// Return a sequence of JSO [k,v] sequences, one for each (k,v) pair in
// the source map `m`, in intrinsic key order of the original GDValues.
static GDValue gdvMapToSequenceOfJSOSequences(GDValue const &m)
{
  GDValue ret(GDVK_SEQUENCE);

  for (GDVMapEntry const &srcKV : m.mapIterableC()) {
    ret.sequenceAppend(
      GDValue(GDVSequence{gdvToJSO(srcKV.first),
                          gdvToJSO(srcKV.second)})
    );
  }

  return ret;
}


// Transform a GDValue map to a JSO map, subject to the assumption that
// `allMapKeysAreStrings(gdvMap)`, so we only need to transform the
// values.
static GDValue gdvMapToJSOMap(GDValue const &gdvMap)
{
  GDValue ret(GDVK_MAP);

  for (GDVMapEntry const &gdvKV : gdvMap.mapIterableC()) {
    ret.mapSetValueAt(gdvKV.first,    // assumed string, no need to transform
                      gdvToJSO(gdvKV.second));
  }

  return ret;
}


// Return a sequence of JSO [k,v] sequences, one for each (k,v) pair in
// the source map `m`, in extrinsic order.
static GDValue gdvOrderedMapToSequenceOfJSOSequences(GDValue const &m)
{
  GDValue ret(GDVK_SEQUENCE);

  for (GDVMapEntry const &srcKV : m.orderedMapIterableC()) {
    ret.sequenceAppend(
      GDValue(GDVSequence{gdvToJSO(srcKV.first),
                          gdvToJSO(srcKV.second)})
    );
  }

  return ret;
}


GDValue gdvToJSO(GDValue const &src)
{
  switch (src.getKind()) {
    default:
      xfailure("bad GDValue kind");
      return GDValue();     // Not reached.

    case GDVK_SYMBOL: {
      if (src.isNull() || src.isBool()) {
        // The special symbols can be serialized as GDVN.
        return src;
      }

      std::string_view name = src.symbolGetName();
      return GDValue(GDVMap{
        { "_type", "symbol" },
        { "value", GDVString(name) },
      });
    }

    case GDVK_INTEGER:
    case GDVK_SMALL_INTEGER: {
      GDVInteger i = src.integerGet();
      if (MOST_NEGATIVE_JSON_INT <= i && i <= MOST_POSITIVE_JSON_INT) {
        // Use it unchanged.
        return src;
      }
      else {
        return GDValue(GDVMap{
          { "_type", "integer" },
          { "value", GDVString(src.asString()) },
        });
      }
    }

    case GDVK_STRING:
      // No transformation needed.
      return src;

    case GDVK_SEQUENCE:
      return gdvSequenceToJSOSequence(src);

    case GDVK_TAGGED_SEQUENCE:
      return GDValue(GDVMap{
        { "_type", "sequence" },
        { "tag", GDVString(src.taggedContainerGetTagName()) },
        { "elements", gdvSequenceToJSOSequence(src) },
      });

    case GDVK_TUPLE:
      return GDValue(GDVMap{
        { "_type", "tuple" },
        { "elements", gdvTupleToJSOSequence(src) },
      });

    case GDVK_TAGGED_TUPLE:
      return GDValue(GDVMap{
        { "_type", "tuple" },
        { "tag", GDVString(src.taggedContainerGetTagName()) },
        { "elements", gdvTupleToJSOSequence(src) },
      });

    case GDVK_SET:
      return GDValue(GDVMap{
        { "_type", "set" },
        { "elements", gdvSetToJSOSequence(src) },
      });

    case GDVK_TAGGED_SET:
      return GDValue(GDVMap{
        { "_type", "set" },
        { "tag", GDVString(src.taggedContainerGetTagName()) },
        { "elements", gdvSetToJSOSequence(src) },
      });

    case GDVK_MAP:
      if (src.containerIsEmpty()) {
        // An empty map would normally be encoded as "{:}" in GDVN,
        // but that is not valid JSON.  So, encode it instead as an
        // empty set, denoted "{}", which means an empty map in JSON.
        return GDValue(GDVSet{});
      }

      // If some keys are not strings, then a direct translation to JSON
      // is not possible.
      if (!allMapKeysAreStrings(src)) {
        return GDValue(GDVMap{
          { "_type", "map" },
          { "elements", gdvMapToSequenceOfJSOSequences(src) },
        });
      }

      return gdvMapToJSOMap(src);

    case GDVK_TAGGED_MAP:
      return GDValue(GDVMap{
        { "_type", "map" },
        { "tag", GDVString(src.taggedContainerGetTagName()) },
        { "elements", gdvMapToSequenceOfJSOSequences(src) },
      });

    case GDVK_ORDERED_MAP:
      return GDValue(GDVMap{
        { "_type", "ordered map" },
        { "elements", gdvOrderedMapToSequenceOfJSOSequences(src) },
      });

    case GDVK_TAGGED_ORDERED_MAP:
      return GDValue(GDVMap{
        { "_type", "ordered map" },
        { "tag", GDVString(src.taggedContainerGetTagName()) },
        { "elements", gdvOrderedMapToSequenceOfJSOSequences(src) },
      });
  }

  // Not reached.
}


std::string jsoToJSON(GDValue const &v, GDValueWriteOptions opts)
{
  opts.m_writeJSON = true;
  return v.asString(opts);
}


std::string gdvToJSON(GDValue const &v, GDValueWriteOptions opts)
{
  return jsoToJSON(gdvToJSO(v), opts);
}


// ---------------------- Convert JSON to GDValue ----------------------
// If `jso` is a pointer to a map that has "tag" mapped to a string,
// return the symbol with the same name.  Otherwise return nullopt.
static std::optional<GDVSymbol> getJSOTagOpt(
  GDValue const * NULLABLE jso)
{
  if (jso && jso->isMap()) {
    if (jso->mapContains("tag")) {
      GDValue const &t = jso->mapGetValueAt("tag");
      if (t.isString()) {
        // Found a string under the right key.
        return { GDVSymbol(t.stringGet()) };
      }
    }
  }

  return {};
}


// Convert a sequence of JSO to a sequence of GDValue.  If
// `jsoContainer` is tagged, create a tagged sequence.
static GDValue jsoSequenceToGDVSequence(
  GDValue const &jsoSeq,
  GDValue const * NULLABLE jsoContainer)
{
  std::optional<GDVSymbol> tagOpt = getJSOTagOpt(jsoContainer);
  GDValue ret(tagOpt? GDVK_TAGGED_SEQUENCE : GDVK_SEQUENCE);
  if (tagOpt) {
    ret.taggedContainerSetTag(*tagOpt);
  }

  for (GDValue const &jsoElt : jsoSeq.sequenceIterableC()) {
    ret.sequenceAppend(jsoToGDV(jsoElt));
  }

  return ret;
}


// Given a sequence of JSO, return a tuple of its elements.  If
// `jsoContainer` is tagged, create a tagged tuple.
static GDValue jsoSequenceToGDVTuple(
  GDValue const &jsoSeq,
  GDValue const * NULLABLE jsoContainer)
{
  std::optional<GDVSymbol> tagOpt = getJSOTagOpt(jsoContainer);
  GDValue ret(tagOpt? GDVK_TAGGED_TUPLE : GDVK_TUPLE);
  if (tagOpt) {
    ret.taggedContainerSetTag(*tagOpt);
  }

  for (GDValue const &element : jsoSeq.sequenceIterableC()) {
    ret.tupleAppend(jsoToGDV(element));
  }

  return ret;
}


// Return a set containing all the elements in `sequence`.  If there are
// duplicate elements, the duplicate inserts will be ignored, so the
// final set only has the unique elements.  If `jsoContainer` is tagged,
// create a tagged set.
static GDValue jsoSequenceToGDVSet(
  GDValue const &jsoSeq,
  GDValue const * NULLABLE jsoContainer)
{
  std::optional<GDVSymbol> tagOpt = getJSOTagOpt(jsoContainer);
  GDValue ret(tagOpt? GDVK_TAGGED_SET : GDVK_SET);
  if (tagOpt) {
    ret.taggedContainerSetTag(*tagOpt);
  }

  for (GDValue const &element : jsoSeq.sequenceIterableC()) {
    ret.setInsert(jsoToGDV(element));
  }

  return ret;
}


// True if `sequence` is a sequence where every element is a sequence of
// exactly two elements.
static bool isSequenceOfTwoElementSequences(GDValue const &sequence)
{
  if (sequence.isSequence()) {
    for (GDValue const &element : sequence.sequenceIterableC()) {
      if (element.isSequence() && element.containerSize() == 2) {
        // Satisfies the condition.
      }
      else {
        return false;
      }
    }

    // All elements satisfy the condition.
    return true;
  }

  else {
    // Not a sequence.
    return false;
  }
}


// Given that `isSequenceOfTwoElementSequences(outerSeq)` is true,
// return a possibly-ordered (depending on `gdvTaggedKind` and
// `gdvUntaggedKind`), possibly-tagged (depending on `jsoContainer`) map
// where each key is the converted (JSO->GDV) first element of one of
// the inner sequences and each associated value is the converted second
// element of the corresponding inner sequence.
//
// If `gdv*Kind` implies an ordered map, the order of `outerSeq` is
// preserved as the extrinsic order of the resulting ordered map.
//
// If `jsoContainer` is tagged, create a tagged map.
//
// If two first elements are equal (after conversion), discard the
// second or later sequence that has a duplicate first element.
//
static GDValue sequenceOfTwoElementJSOSequencesToGDVPOMap(
  GDValueKind gdvTaggedKind,
  GDValueKind gdvUntaggedKind,
  GDValue const &outerSeq,
  GDValue const * NULLABLE jsoContainer)
{
  std::optional<GDVSymbol> tagOpt = getJSOTagOpt(jsoContainer);
  GDValue ret(tagOpt? gdvTaggedKind : gdvUntaggedKind);
  xassert(ret.isPOMap());
  if (tagOpt) {
    ret.taggedContainerSetTag(*tagOpt);
  }

  for (GDValue const &innerSeq : outerSeq.sequenceIterableC()) {
    GDValue const &jsoKey = innerSeq.sequenceGetValueAt(0);
    GDValue const &jsoValue = innerSeq.sequenceGetValueAt(1);

    GDValue gdvKey = jsoToGDV(jsoKey);

    if (!ret.mapContains(gdvKey)) {
      ret.mapSetValueAt(gdvKey, jsoToGDV(jsoValue));
    }
  }

  return ret;
}

static GDValue sequenceOfTwoElementJSOSequencesToGDVMap(
  GDValue const &outerSeq,
  GDValue const * NULLABLE jsoContainer)
{
  return sequenceOfTwoElementJSOSequencesToGDVPOMap(
    GDVK_TAGGED_MAP, GDVK_MAP, outerSeq, jsoContainer);
}

static GDValue sequenceOfTwoElementJSOSequencesToGDVOrderedMap(
  GDValue const &outerSeq,
  GDValue const * NULLABLE jsoContainer)
{
  return sequenceOfTwoElementJSOSequencesToGDVPOMap(
    GDVK_TAGGED_ORDERED_MAP, GDVK_ORDERED_MAP, outerSeq, jsoContainer);
}


// Given a JSO map, convert it to a GDValue map by transforming the
// values from JSO to GDV.
static GDValue jsoMapToGDVMap(GDValue const &jsoMap)
{
  GDValue ret(GDVK_MAP);

  for (GDVMapEntry const &jsoKV : jsoMap.mapIterableC()) {
    ret.mapSetValueAt(jsoKV.first,     // assumed to be a string
                      jsoToGDV(jsoKV.second));
  }

  return ret;
}


GDValue jsoToGDV(GDValue const &src)
{
  switch (src.getKind()) {
    case GDVK_SEQUENCE:
      return jsoSequenceToGDVSequence(src, nullptr);

    case GDVK_SET:
      if (src.containerIsEmpty()) {
        // Above, we turn an empty map into an empty set for JSON.  Now,
        // invert that.
        return GDVMap{};
      }
      else {
        // This is unexpected, since sets should always be turned into
        // maps for JSON, but at least for now, I'm just going to let it
        // through.
      }
      break;

    case GDVK_MAP:
      if (src.mapContains("_type")) {
        GDValue const &t = src.mapGetValueAt("_type");
        if (t.isString()) {
          GDVString const &tname = t.stringGet();
          if (tname == "symbol") {
            if (src.mapContains("value")) {
              GDValue const &v = src.mapGetValueAt("value");
              if (v.isString()) {
                // Encoded symbol.
                return GDVSymbol(v.stringGet());
              }
            }
          }

          else if (tname == "integer") {
            if (src.mapContains("value")) {
              GDValue const &v = src.mapGetValueAt("value");
              if (v.isString()) {
                // Encoded large integer.
                return GDVInteger::fromDigits(v.stringGet());
              }
            }
          }

          else if (tname == "sequence") {
            if (src.mapContains("elements")) {
              GDValue const &v = src.mapGetValueAt("elements");
              if (v.isSequence()) {
                // Encoded sequence.
                return jsoSequenceToGDVSequence(v, &src);
              }
            }
          }

          else if (tname == "tuple") {
            if (src.mapContains("elements")) {
              GDValue const &v = src.mapGetValueAt("elements");
              if (v.isSequence()) {
                // Encoded tuple.
                return jsoSequenceToGDVTuple(v, &src);
              }
            }
          }

          else if (tname == "set") {
            if (src.mapContains("elements")) {
              GDValue const &v = src.mapGetValueAt("elements");
              if (v.isSequence()) {
                // Encoded set.
                return jsoSequenceToGDVSet(v, &src);
              }
            }
          }

          else if (tname == "map") {
            if (src.mapContains("elements")) {
              GDValue const &v = src.mapGetValueAt("elements");
              if (isSequenceOfTwoElementSequences(v)) {
                // Encoded map.
                return sequenceOfTwoElementJSOSequencesToGDVMap(v, &src);
              }
            }
          }

          else if (tname == "ordered map") {
            if (src.mapContains("elements")) {
              GDValue const &v = src.mapGetValueAt("elements");
              if (isSequenceOfTwoElementSequences(v)) {
                // Encoded ordered map.
                return sequenceOfTwoElementJSOSequencesToGDVOrderedMap(v, &src);
              }
            }
          }
        }
      }

      // No special type.  But we still need to transform the values.
      return jsoMapToGDVMap(src);

    default:
      // Others can be kept as-is.
      break;
  }

  // If we do not recognize a special form, return as-is.
  return src;
}


GDValue jsonToGDV(std::string_view json)
{
  return jsoToGDV(GDValue::readFromStringView(json));
}


CLOSE_NAMESPACE(gdv)


// EOF
