// xarithmetic.cc
// Code for `xarithmetic.h`.

#include "xarithmetic.h"               // this module


OPEN_NAMESPACE(smbase)


// create-tuple-class: definitions for XDivideByZero
/*AUTO_CTC*/ XDivideByZero::XDivideByZero(
/*AUTO_CTC*/   std::string const &dividend)
/*AUTO_CTC*/   : XArithmetic(),
/*AUTO_CTC*/     m_dividend(dividend)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XDivideByZero::XDivideByZero(XDivideByZero const &obj) noexcept
/*AUTO_CTC*/   : XArithmetic(obj),
/*AUTO_CTC*/     DMEMB(m_dividend)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XDivideByZero &XDivideByZero::operator=(XDivideByZero const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XArithmetic::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_dividend);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XDivideByZero::getConflict() const
{
  return stringb(
    "Attempt to divide " << m_dividend << " by zero.");
}


CLOSE_NAMESPACE(smbase)


// EOF
