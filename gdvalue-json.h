// gdvalue-json.h
// Convert between GDValues and JSON.

#ifndef SMBASE_GDVALUE_JSON_H
#define SMBASE_GDVALUE_JSON_H

#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue
#include "smbase/gdvalue-write-options.h"        // GDValueWriteOptions
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, NULLABLE
#include "smbase/std-string-view-fwd.h"          // std::string_view

#include <cstdint>                               // std::int64_t


OPEN_NAMESPACE(gdv)


// The most positive and most negative integer values that can be safely
// encoded in JSON using integer notation.
//
// TODO: These are not the correct values.
std::int64_t const MOST_POSITIVE_JSON_INT =   999999999;
std::int64_t const MOST_NEGATIVE_JSON_INT = -1000000000;


// ---------------------- Convert GDValue to JSON ----------------------
// The first step in converting GDValue to JSON is to convert it to a
// subset of GDValue that might be called "JSO", the semantic subset of
// GDV that corresponds to what can be expressed as JSON.
GDValue gdvToJSO(GDValue const &src);

// The next step is to serialize the JSO using JSON syntactc
// conventions, most notably adding commas between values.  Internally,
// this enables `opts.m_writeJSON`.
std::string jsoToJSON(GDValue const &v, GDValueWriteOptions opts = {});

// These two steps can be combined into one.
std::string gdvToJSON(GDValue const &v, GDValueWriteOptions opts = {});


// ---------------------- Convert JSON to GDValue ----------------------
// Since JSON is a subset of GDVN, the first step of deserializing is to
// treat it as GDVN and read a GDValue.  This is the second step,
// inverting the transformations that `gdvToJSO` did to conform to the
// JSON data model.  This does not report any errors; it will invert
// recognized forms, while returning unrecognized forms unchanged.
GDValue jsoToGDV(GDValue const &src);

// Convert `json` to JSO, then to GDValue.
GDValue jsonToGDV(std::string_view json);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_JSON_H
