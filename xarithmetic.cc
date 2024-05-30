// xarithmetic.cc
// Code for `xarithmetic.h`.

#include "xarithmetic.h"               // this module


// create-tuple-class: definitions for XDivideByZero
/*AUTO_CTC*/ XDivideByZero::XDivideByZero(
/*AUTO_CTC*/   std::string const &dividend)
/*AUTO_CTC*/   : XArithmetic(),
/*AUTO_CTC*/     m_dividend(dividend)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XDivideByZero::XDivideByZero(XDivideByZero const &obj)
/*AUTO_CTC*/   : XArithmetic(obj),
/*AUTO_CTC*/     DMEMB(m_dividend)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XDivideByZero::XDivideByZero(XDivideByZero &&obj)
/*AUTO_CTC*/   : XArithmetic(std::move(obj)),
/*AUTO_CTC*/     MDMEMB(m_dividend)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XDivideByZero &XDivideByZero::operator=(XDivideByZero const &obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XArithmetic::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_dividend);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ XDivideByZero &XDivideByZero::operator=(XDivideByZero &&obj)
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XArithmetic::operator=(std::move(obj));
/*AUTO_CTC*/     MCMEMB(m_dividend);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XDivideByZero::getConflict() const
{
  return stringb(
    "Attempt to divide " << m_dividend << " by zero.");
}


// EOF
