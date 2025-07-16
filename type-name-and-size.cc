// type-name-and-size.cc
// Code for `type-name-and-size.h`.

// See license.txt for copyright and terms of use.

#include "smbase/type-name-and-size.h" // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/stringb.h"            // stringb

#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// create-tuple-class: definitions for TypeNameAndSize
/*AUTO_CTC*/ TypeNameAndSize::TypeNameAndSize(
/*AUTO_CTC*/   std::string const &name,
/*AUTO_CTC*/   int bits)
/*AUTO_CTC*/   : m_name(name),
/*AUTO_CTC*/     m_bits(bits)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ TypeNameAndSize::TypeNameAndSize(
/*AUTO_CTC*/   std::string &&name,
/*AUTO_CTC*/   int bits)
/*AUTO_CTC*/   : m_name(name),
/*AUTO_CTC*/     m_bits(bits)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ TypeNameAndSize::TypeNameAndSize(TypeNameAndSize const &obj) noexcept
/*AUTO_CTC*/   : DMEMB(m_name),
/*AUTO_CTC*/     DMEMB(m_bits)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ TypeNameAndSize::TypeNameAndSize(TypeNameAndSize &&obj) noexcept
/*AUTO_CTC*/   : MDMEMB(m_name),
/*AUTO_CTC*/     MDMEMB(m_bits)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ TypeNameAndSize &TypeNameAndSize::operator=(TypeNameAndSize const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_name);
/*AUTO_CTC*/     CMEMB(m_bits);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/
/*AUTO_CTC*/ TypeNameAndSize &TypeNameAndSize::operator=(TypeNameAndSize &&obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     MCMEMB(m_name);
/*AUTO_CTC*/     MCMEMB(m_bits);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string TypeNameAndSize::toString() const
{
  return stringb(doubleQuote(m_name) << " (" << m_bits << " bits)");
}


CLOSE_NAMESPACE(smbase)


// EOF
