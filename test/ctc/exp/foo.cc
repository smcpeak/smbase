// foo.cc
// Code for `foo.h`.

#include "foo.h"                       // this module

#include "compare-util.h"              // RET_IF_COMPARE_MEMBERS
#include "gdvalue-parser.h"            // gdv::GDValueParser
#include "gdvalue.h"                   // gdv::GDValue
#include "sm-macros.h"                 // DMEMB, CMEMB, etc.

#include <iostream>                    // std::ostream
#include <sstream>                     // std::ostringstream
#include <utility>                     // std::move

// For `smbase::compare`.
using namespace smbase;


// ---- create-tuple-class: definitions for Foo
/*AUTO_CTC*/ Foo::Foo(
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
/*AUTO_CTC*/ Foo::Foo(
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
/*AUTO_CTC*/ Foo::Foo(Foo const &obj) noexcept
/*AUTO_CTC*/   : DMEMB(m_x),
/*AUTO_CTC*/     DMEMB(m_y),
/*AUTO_CTC*/     DMEMB(m_z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo::Foo(Foo &&obj) noexcept
/*AUTO_CTC*/   : MDMEMB(m_x),
/*AUTO_CTC*/     MDMEMB(m_y),
/*AUTO_CTC*/     MDMEMB(m_z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   selfCheck();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo &Foo::operator=(Foo const &obj) noexcept
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
/*AUTO_CTC*/ Foo &Foo::operator=(Foo &&obj) noexcept
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
/*AUTO_CTC*/ int compare(Foo const &a, Foo const &b)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_x);
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_y);
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_z);
/*AUTO_CTC*/   return 0;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::string Foo::toString() const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   std::ostringstream oss;
/*AUTO_CTC*/   write(oss);
/*AUTO_CTC*/   return oss.str();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::ostream &operator<<(std::ostream &os, Foo const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   obj.write(os);
/*AUTO_CTC*/   return os;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo::operator gdv::GDValue() const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   using namespace gdv;
/*AUTO_CTC*/   GDValue m(GDVK_TAGGED_ORDERED_MAP, "Foo"_sym);
/*AUTO_CTC*/   GDV_WRITE_MEMBER_SYM(m_x);
/*AUTO_CTC*/   GDV_WRITE_MEMBER_SYM(m_y);
/*AUTO_CTC*/   GDV_WRITE_MEMBER_SYM(m_z);
/*AUTO_CTC*/   return m;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo::Foo(gdv::GDValueParser const &p)
/*AUTO_CTC*/   : GDVP_READ_MEMBER_SYM(m_x),
/*AUTO_CTC*/     GDVP_READ_MEMBER_SYM(m_y),
/*AUTO_CTC*/     GDVP_READ_MEMBER_SYM(m_z)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   p.checkTaggedOrderedMapTag("Foo");
/*AUTO_CTC*/ }
/*AUTO_CTC*/


// ---- create-tuple-class: definitions for Bar
/*AUTO_CTC*/ Bar::Bar(
/*AUTO_CTC*/   int n)
/*AUTO_CTC*/   : EmptyBase(),
/*AUTO_CTC*/     IMEMBFP(n)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Bar::Bar(Bar const &obj) noexcept
/*AUTO_CTC*/   : EmptyBase(obj),
/*AUTO_CTC*/     DMEMB(m_n)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Bar &Bar::operator=(Bar const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     EmptyBase::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_n);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ int compare(Bar const &a, Bar const &b)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   RET_IF_COMPARE_MEMBERS(m_n);
/*AUTO_CTC*/   return 0;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::string Bar::toString() const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   std::ostringstream oss;
/*AUTO_CTC*/   write(oss);
/*AUTO_CTC*/   return oss.str();
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ void Bar::write(std::ostream &os) const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   os << "{";
/*AUTO_CTC*/   WRITE_MEMBER(m_n);
/*AUTO_CTC*/   os << " }";
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::ostream &operator<<(std::ostream &os, Bar const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   obj.write(os);
/*AUTO_CTC*/   return os;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


// ---- create-tuple-class: definitions for Baz
/*AUTO_CTC*/ Baz::Baz(
/*AUTO_CTC*/   int *p)
/*AUTO_CTC*/   : IMEMBFP(p)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Baz::Baz(Baz const &obj) noexcept
/*AUTO_CTC*/   : DMEMB(m_p)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Baz &Baz::operator=(Baz const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_p);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


// EOF
