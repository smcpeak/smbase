// ctc-test.cc
// Code for `ctc-test`.

#include "smbase/ctc-test.h"                     // my decls

#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE, DMEMB, CMEMB, etc.
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/compare-util.h"                 // RET_IF_COMPARE_MEMBERS
#include "smbase/gdvalue-parser.h"               // gdv::GDValueParser
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/gdvn-test-roundtrip.h"          // gdvnTestRoundtripEq

#include <iostream>                              // std::ostream
#include <sstream>                               // std::ostringstream
#include <utility>                               // std::move

using namespace smbase;


// ------------------------------ CTCTest ------------------------------
// ---- create-tuple-class: definitions for CTCTest
/*AUTO_CTC*/ CTCTest::CTCTest(
/*AUTO_CTC*/   int x,
/*AUTO_CTC*/   float y,
/*AUTO_CTC*/   std::string const &z)
/*AUTO_CTC*/   : IMEMBFP(x),
/*AUTO_CTC*/     IMEMBFP(y),
/*AUTO_CTC*/     IMEMBFP(z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest::CTCTest(
/*AUTO_CTC*/   int x,
/*AUTO_CTC*/   float y,
/*AUTO_CTC*/   std::string &&z)
/*AUTO_CTC*/   : IMEMBMFP(x),
/*AUTO_CTC*/     IMEMBMFP(y),
/*AUTO_CTC*/     IMEMBMFP(z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest::CTCTest(CTCTest const &obj) noexcept
/*AUTO_CTC*/   : DMEMB(m_x),
/*AUTO_CTC*/     DMEMB(m_y),
/*AUTO_CTC*/     DMEMB(m_z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest::CTCTest(CTCTest &&obj) noexcept
/*AUTO_CTC*/   : MDMEMB(m_x),
/*AUTO_CTC*/     MDMEMB(m_y),
/*AUTO_CTC*/     MDMEMB(m_z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest &CTCTest::operator=(CTCTest const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_x);
/*AUTO_CTC*/     CMEMB(m_y);
/*AUTO_CTC*/     CMEMB(m_z);
/*AUTO_CTC*/     selfCheck();
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest &CTCTest::operator=(CTCTest &&obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     MCMEMB(m_x);
/*AUTO_CTC*/     MCMEMB(m_y);
/*AUTO_CTC*/     MCMEMB(m_z);
/*AUTO_CTC*/     selfCheck();
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ int compare(CTCTest const &a, CTCTest const &b)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_x);
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_y);
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_z);
/*AUTO_CTC*/   return 0;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::string CTCTest::toString() const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   std::ostringstream oss;
/*AUTO_CTC*/   write(oss);
/*AUTO_CTC*/   return oss.str();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::ostream &operator<<(std::ostream &os, CTCTest const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   obj.write(os);
/*AUTO_CTC*/   return os;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest::operator gdv::GDValue() const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   using namespace gdv;
/*AUTO_CTC*/   GDValue m(GDVK_TAGGED_ORDERED_MAP, "CTCTest"_sym);
/*AUTO_CTC*/   GDV_WRITE_MEMBER_SYM(m_x);
/*AUTO_CTC*/   GDV_WRITE_MEMBER_SYM(m_y);
/*AUTO_CTC*/   GDV_WRITE_MEMBER_SYM(m_z);
/*AUTO_CTC*/   return m;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ void CTCTest::write(std::ostream &os) const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   operator gdv::GDValue().writeIndented(os);
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ CTCTest::CTCTest(gdv::GDValueParser const &p)
/*AUTO_CTC*/   : GDVP_READ_MEMBER_SYM(m_x),
/*AUTO_CTC*/     GDVP_READ_MEMBER_SYM(m_y),
/*AUTO_CTC*/     GDVP_READ_MEMBER_SYM(m_z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   p.checkTaggedOrderedMapTag("CTCTest");
/*AUTO_CTC*/ }
/*AUTO_CTC*/


void CTCTest::selfCheck() const
{}


// ------------------------------- Tests -------------------------------
OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  CTCTest const ctcTest(
    3,
    4.5,
    "some string");

  // Check GDV de/serialization.
  gdvnTestRoundtripEq(ctcTest,
    "CTCTest["
      "x:3 "
      "y:4.5 "
      "z:\"some string\""
    "]");

  // Check `toString()`, which should now use GDVN.
  EXPECT_EQ(ctcTest.toString(), "CTCTest["
    "x:3 "
    "y:4.5 "
    "z:\"some string\""
  "]");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_ctc()
{
  test_basics();
}


// EOF
