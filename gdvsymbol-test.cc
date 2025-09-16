// gdvsymbol-test.cc
// Tests for `gdvsymbol` module.

// This file is in the public domain.

#include "gdvsymbol.h"                 // module under test

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <limits>                      // std::numeric_limits
#include <string_view>                 // std::string_view

using namespace gdv;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  GDVSymbol s1;
  EXPECT_EQ(s1.getSymbolName(), "null");
  EXPECT_EQ(s1.size(), 4);
  EXPECT_EQ(s1.getSymbolIndex(), 0);
  EXPECT_EQ(stringb(s1), "null");
  EXPECT_EQ(s1.asString(), "null");
  EXPECT_EQ(s1.asString(false /*forceQuotes*/), "null");
  EXPECT_EQ(s1.asString(true /*forceQuotes*/), "`null`");
  EXPECT_EQ(s1.quotedString(), "`null`");

  GDVSymbol s2("hello");
  EXPECT_EQ(s2.getSymbolName(), "hello");
  EXPECT_EQ(s2.size(), 5);
  EXPECT_EQ(stringb(s2), "hello");
  EXPECT_EQ(s2.asString(), "hello");
  EXPECT_EQ(s2.quotedString(), "`hello`");
  xassert(s1 > s2);
  xassert(GDVSymbol::compareIndices(s1.getSymbolIndex(),
                                    s2.getSymbolIndex()) > 0);

  s1.swap(s2);

  EXPECT_EQ(s2.getSymbolName(), "null");
  EXPECT_EQ(s1.getSymbolName(), "hello");
  xassert(s1 < s2);

  GDVSymbol s3(GDVSymbol::DirectIndex, s1.getSymbolIndex());
  EXPECT_EQ(s3.getSymbolName(), "hello");
  EXPECT_EQ(s1, s3);
  EXPECT_EQ(compare(s1, s3), 0);

  xassert(!GDVSymbol::validUnquotedSymbolName(""));
  xassert(GDVSymbol::validUnquotedSymbolName("_"));
  xassert(GDVSymbol::validUnquotedSymbolName("_9"));
  xassert(!GDVSymbol::validUnquotedSymbolName("9"));
  xassert(GDVSymbol::validUnquotedSymbolName("a9"));
  xassert(!GDVSymbol::validUnquotedSymbolName("a!"));
  xassert(!GDVSymbol::validUnquotedSymbolName("!"));

  EXPECT_EQ(GDVSymbol("has spaces").asString(), "`has spaces`");
  EXPECT_EQ(GDVSymbol("has spaces").quotedString(), "`has spaces`");
}


void test_literal_and_equality()
{
  auto a = "alpha"_sym;
  GDVSymbol b("alpha");

  // Literal operator should behave like constructor from string.
  EXPECT_EQ(a.getSymbolName(), "alpha");
  EXPECT_EQ(b.getSymbolName(), "alpha");
  EXPECT_EQ(a, b);

  // Same index and ordering should be equal.
  EXPECT_EQ(compare(a, b), 0);
  xassert(!(a != b));

  // Names and sizes consistent.
  EXPECT_EQ(a.size(), a.getSymbolName().size());
  EXPECT_EQ(b.size(), b.getSymbolName().size());
}


void test_lookup_indices_and_validIndex()
{
  // lookupSymbolIndex should be idempotent.
  auto i1 = GDVSymbol::lookupSymbolIndex("beta");
  auto i2 = GDVSymbol::lookupSymbolIndex("beta");
  EXPECT_EQ(i1, i2);
  xassert(GDVSymbol::validIndex(i1));

  // Construct by direct index and verify the name is retrieved.
  GDVSymbol sBeta(GDVSymbol::DirectIndex, i1);
  EXPECT_EQ(sBeta.getSymbolName(), "beta");

  // Null index behavior.
  auto nullIdx = GDVSymbol::getNullSymbolIndex();
  GDVSymbol sNull;
  EXPECT_EQ(sNull.getSymbolIndex(), nullIdx);
  xassert(GDVSymbol::validIndex(nullIdx));

  // "null" should also be in the table at the null index.
  auto lookedUpNull = GDVSymbol::lookupSymbolIndex("null");
  EXPECT_EQ(lookedUpNull, nullIdx);

  // A very large index should not be valid since validity requires that
  // the index be assigned.
  using I = GDVSymbol::Index;
  xassert(!GDVSymbol::validIndex(std::numeric_limits<I>::max()));

  // Negatives are also not valid.
  xassert(!GDVSymbol::validIndex(-1));
}


void test_compare_and_relational()
{
  GDVSymbol a("a");
  GDVSymbol b("b");
  GDVSymbol A("A");

  // Lexicographic ordering by contents.
  xassert(a < b);
  xassert(compare(a, b) < 0);
  xassert(compare(b, a) > 0);
  xassert(!(a == b));
  xassert(a <= b);
  xassert(b >= a);

  // ASCII ordering: 'A' < 'a'
  xassert(A < a);
  xassert(compare(A, a) < 0);

  // Indices comparison is consistent with symbol comparison.
  auto ia = a.getSymbolIndex();
  auto ib = b.getSymbolIndex();
  xassert(GDVSymbol::compareIndices(ia, ib) < 0);
  xassert(GDVSymbol::compareIndices(ib, ia) > 0);
  xassert(GDVSymbol::compareIndices(ia, ia) == 0);

  // Another pair to double-check.
  GDVSymbol s1("aardvark");
  GDVSymbol s2("zebra");
  xassert(compare(s1, s2) < 0);
  xassert(GDVSymbol::compareIndices(s1.getSymbolIndex(), s2.getSymbolIndex()) < 0);
}


void test_asString_quoting_and_valid_names()
{
  // Valid unquoted name.
  GDVSymbol v("_abc123");
  xassert(GDVSymbol::validUnquotedSymbolName(v.getSymbolName()));
  EXPECT_EQ(v.asString(), "_abc123");
  EXPECT_EQ(v.asString(true), "`_abc123`");   // forceQuotes should quote even valid names.
  EXPECT_EQ(v.quotedString(), "`_abc123`");

  // Starts with digit -> must be quoted.
  GDVSymbol d("9lives");
  xassert(!GDVSymbol::validUnquotedSymbolName(d.getSymbolName()));
  EXPECT_EQ(d.asString(), "`9lives`");
  EXPECT_EQ(d.asString(true), "`9lives`");    // same whether forced or not.

  // Contains hyphen -> must be quoted.
  GDVSymbol h("a-b");
  xassert(!GDVSymbol::validUnquotedSymbolName(h.getSymbolName()));
  EXPECT_EQ(h.asString(), "`a-b`");
  EXPECT_EQ(h.asString(true), "`a-b`");

  // Contains space -> already tested elsewhere; recheck forceQuotes.
  GDVSymbol sp("two words");
  xassert(!GDVSymbol::validUnquotedSymbolName(sp.getSymbolName()));
  EXPECT_EQ(sp.asString(false), "`two words`");
  EXPECT_EQ(sp.asString(true), "`two words`");

  // Additional valid patterns.
  xassert(GDVSymbol::validUnquotedSymbolName("A_b0"));
  xassert(GDVSymbol::validUnquotedSymbolName("a_b_c123"));
  xassert(!GDVSymbol::validUnquotedSymbolName("x y"));
  xassert(!GDVSymbol::validUnquotedSymbolName("a-b"));
}


void test_stream_output_operator()
{
  // operator<< mirrors asString(false).
  EXPECT_EQ(stringb("_abc"_sym), "_abc");            // valid name -> no quotes
  EXPECT_EQ(stringb(GDVSymbol("a-b")), "`a-b`");     // invalid name -> backticks
  EXPECT_EQ(stringb(GDVSymbol("_ok_123")), "_ok_123");
}


void test_swap_and_copy()
{
  GDVSymbol g("gamma");
  GDVSymbol gCopy = g;
  EXPECT_EQ(gCopy, g);

  // swap with another
  GDVSymbol z("zeta");
  xassert(g != z);
  g.swap(z);
  EXPECT_EQ(g.getSymbolName(), "zeta");
  EXPECT_EQ(z.getSymbolName(), "gamma");

  // swap back
  swap(g, z);
  EXPECT_EQ(g, gCopy);

  // self-swap is a no-op
  g.swap(g);
  EXPECT_EQ(g, gCopy);
}


void test_name_size_consistency()
{
  GDVSymbol n("needle");
  EXPECT_EQ(n.size(), 6u);
  EXPECT_EQ(n.size(), n.getSymbolName().size());

  // Also verify for default/null symbol.
  GDVSymbol sNull;
  EXPECT_EQ(sNull.getSymbolName(), "null");
  EXPECT_EQ(sNull.size(), sNull.getSymbolName().size());
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvsymbol()
{
  test_basics();
  test_literal_and_equality();
  test_lookup_indices_and_validIndex();
  test_compare_and_relational();
  test_asString_quoting_and_valid_names();
  test_stream_output_operator();
  test_swap_and_copy();
  test_name_size_consistency();
}


// EOF
