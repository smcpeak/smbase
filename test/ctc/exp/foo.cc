// foo.cc
// Code for `foo.h`.

#include "foo.h"                       // this module

#include "compare-util.h"              // RET_IF_COMPARE_MEMBERS
#include "sm-macros.h"                 // DMEMB, CMEMB, etc.

#include <iostream>                    // std::ostream
#include <sstream>                     // std::ostringstream
#include <utility>                     // std::move


// ---- create-tuple-class: definitions for Foo
/*AUTO_CTC*/ Foo::Foo(int x, float y, std::string const &z)
/*AUTO_CTC*/   : m_x(x),
/*AUTO_CTC*/     m_y(y),
/*AUTO_CTC*/     m_z(z)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo::Foo(Foo const &obj)
/*AUTO_CTC*/   : DMEMB(m_x),
/*AUTO_CTC*/     DMEMB(m_y),
/*AUTO_CTC*/     DMEMB(m_z)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo::Foo(Foo &&obj)
/*AUTO_CTC*/   : MDMEMB(m_x),
/*AUTO_CTC*/     MDMEMB(m_y),
/*AUTO_CTC*/     MDMEMB(m_z)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo &Foo::operator=(Foo const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_x);
/*AUTO_CTC*/     CMEMB(m_y);
/*AUTO_CTC*/     CMEMB(m_z);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Foo &Foo::operator=(Foo &&obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     MCMEMB(m_x);
/*AUTO_CTC*/     MCMEMB(m_y);
/*AUTO_CTC*/     MCMEMB(m_z);
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
/*AUTO_CTC*/ void Foo::write(std::ostream &os) const
/*AUTO_CTC*/ {
/*AUTO_CTC*/   os << "{";
/*AUTO_CTC*/   WRITE_MEMBER(m_x);
/*AUTO_CTC*/   WRITE_MEMBER(m_y);
/*AUTO_CTC*/   WRITE_MEMBER(m_z);
/*AUTO_CTC*/   os << " }";
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ std::ostream &operator<<(std::ostream &os, Foo const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   obj.write(os);
/*AUTO_CTC*/   return os;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


// ---- create-tuple-class: definitions for Bar
/*AUTO_CTC*/ Bar::Bar(int n)
/*AUTO_CTC*/   : EmptyBase(),
/*AUTO_CTC*/     m_n(n)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Bar::Bar(Bar const &obj)
/*AUTO_CTC*/   : EmptyBase(obj),
/*AUTO_CTC*/     DMEMB(m_n)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Bar::Bar(Bar &&obj)
/*AUTO_CTC*/   : EmptyBase(std::move(obj)),
/*AUTO_CTC*/     MDMEMB(m_n)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Bar &Bar::operator=(Bar const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     EmptyBase::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_n);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Bar &Bar::operator=(Bar &&obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     EmptyBase::operator=(std::move(obj));
/*AUTO_CTC*/     MCMEMB(m_n);
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
/*AUTO_CTC*/ Baz::Baz(int *p)
/*AUTO_CTC*/   : m_p(p)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Baz::Baz(Baz const &obj)
/*AUTO_CTC*/   : DMEMB(m_p)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Baz::Baz(Baz &&obj)
/*AUTO_CTC*/   : MDMEMB(m_p)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ Baz &Baz::operator=(Baz const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_p);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ Baz &Baz::operator=(Baz &&obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     MCMEMB(m_p);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


// EOF
