// gdvn-test-roundtrip.h
// Routines for testing GDVN round-trip serialization.

// See license.txt for copyright and terms of use.

// This module is tested primarily by `gdvalue-parser-test`.

#ifndef SMBASE_GDVN_TEST_ROUNDTRIP_H
#define SMBASE_GDVN_TEST_ROUNDTRIP_H

#include "smbase/gdvalue-parser.h"     // gdv::{GDValueParser, gdvpTo}
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-test.h"            // EXPECT_EQ


// Serialize `t` down to GDVN, expecting `expectGDVN`, then deserialize
// back to `T`, and check for equality.
template <typename T>
void gdvnTestRoundtrip(T const &t, char const *expectGDVN)
{
  using gdv::toGDValue;

  // T -> GDV
  gdv::GDValue v = toGDValue(t);

  // GDV -> GDVN
  EXPECT_EQ(v.asString(), expectGDVN);

  // GDVN -> GDV
  EXPECT_EQ(gdv::fromGDVN(expectGDVN), v);

  // GDV -> T
  gdv::GDValueParser p(v);
  T t2(gdv::gdvpTo<T>(p));

  // Check that `t` equals `t2` by checking their GDV serializations.
  EXPECT_EQ(toGDValue(t2), v);
}


// The "Eq" part of the name is meant to indicate that we assume we
// can compare `T` values using `operator==`.  That might not be true
// because the operator is missing, or because (as for `unique_ptr`)
// it does not do what we want here.
template <typename T>
void gdvnTestRoundtripEq(T const &t, char const *expectGDVN)
{
  using gdv::toGDValue;

  // T -> GDV
  gdv::GDValue v = toGDValue(t);

  // GDV -> GDVN
  EXPECT_EQ(v.asString(), expectGDVN);

  // GDVN -> GDV
  EXPECT_EQ(gdv::fromGDVN(expectGDVN), v);

  // GDV -> T
  gdv::GDValueParser p(v);
  T t2 = gdv::gdvpTo<T>(p);

  // Check that `t` equals `t2` by checking their GDV serializations.
  EXPECT_EQ(toGDValue(t2), v);

  // Then check direct equality.  If this fails, we don't necessarily
  // have any way to readily diagnose since we already saw that the GDV
  // forms were the same, and don't want to assume the type has other
  // serialization capabilities (such as writing to `ostream`).
  xassert(t2 == t);
}


#endif // SMBASE_GDVN_TEST_ROUNDTRIP_H
