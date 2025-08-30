// xoverflow.cc
// Code for `xoverflow.h`.

#include "xoverflow.h"                 // this module

#include "sm-macros.h"                 // DMEMB, CMEMB
#include "stringb.h"                   // stringb


OPEN_NAMESPACE(smbase)


// create-tuple-class: definitions for XBinaryOpOverflow
/*AUTO_CTC*/ XBinaryOpOverflow::XBinaryOpOverflow(
/*AUTO_CTC*/   TypeNameAndSize const &type,
/*AUTO_CTC*/   std::string const &lhs,
/*AUTO_CTC*/   std::string const &rhs,
/*AUTO_CTC*/   std::string const &op)
/*AUTO_CTC*/   : XOverflow(),
/*AUTO_CTC*/     IMEMBFP(type),
/*AUTO_CTC*/     IMEMBFP(lhs),
/*AUTO_CTC*/     IMEMBFP(rhs),
/*AUTO_CTC*/     IMEMBFP(op)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XBinaryOpOverflow::XBinaryOpOverflow(XBinaryOpOverflow const &obj) noexcept
/*AUTO_CTC*/   : XOverflow(obj),
/*AUTO_CTC*/     DMEMB(m_type),
/*AUTO_CTC*/     DMEMB(m_lhs),
/*AUTO_CTC*/     DMEMB(m_rhs),
/*AUTO_CTC*/     DMEMB(m_op)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XBinaryOpOverflow &XBinaryOpOverflow::operator=(XBinaryOpOverflow const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XOverflow::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_type);
/*AUTO_CTC*/     CMEMB(m_lhs);
/*AUTO_CTC*/     CMEMB(m_rhs);
/*AUTO_CTC*/     CMEMB(m_op);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XBinaryOpOverflow::getConflict() const
{
  return stringb(
    "Arithmetic overflow of type " << m_type.toString() << ": " <<
    m_lhs << ' ' << m_op << ' ' << m_rhs << " would overflow.");
}


// create-tuple-class: definitions for XNumericConversionNoRTIP
/*AUTO_CTC*/ XNumericConversionNoRTIP::XNumericConversionNoRTIP(
/*AUTO_CTC*/   std::string const &sourceValue,
/*AUTO_CTC*/   std::string const &destValue,
/*AUTO_CTC*/   std::string const &roundTripValue,
/*AUTO_CTC*/   TypeNameAndSize const &sourceType,
/*AUTO_CTC*/   TypeNameAndSize const &destType)
/*AUTO_CTC*/   : XNumericConversion(),
/*AUTO_CTC*/     IMEMBFP(sourceValue),
/*AUTO_CTC*/     IMEMBFP(destValue),
/*AUTO_CTC*/     IMEMBFP(roundTripValue),
/*AUTO_CTC*/     IMEMBFP(sourceType),
/*AUTO_CTC*/     IMEMBFP(destType)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionNoRTIP::XNumericConversionNoRTIP(XNumericConversionNoRTIP const &obj) noexcept
/*AUTO_CTC*/   : XNumericConversion(obj),
/*AUTO_CTC*/     DMEMB(m_sourceValue),
/*AUTO_CTC*/     DMEMB(m_destValue),
/*AUTO_CTC*/     DMEMB(m_roundTripValue),
/*AUTO_CTC*/     DMEMB(m_sourceType),
/*AUTO_CTC*/     DMEMB(m_destType)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionNoRTIP &XNumericConversionNoRTIP::operator=(XNumericConversionNoRTIP const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XNumericConversion::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_sourceValue);
/*AUTO_CTC*/     CMEMB(m_destValue);
/*AUTO_CTC*/     CMEMB(m_roundTripValue);
/*AUTO_CTC*/     CMEMB(m_sourceType);
/*AUTO_CTC*/     CMEMB(m_destType);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XNumericConversionNoRTIP::getConflict() const
{
  return stringb(
    "Source value " << m_sourceValue <<
    " of type " << m_sourceType.toString() <<
    ", when converted to destination type " << m_destType.toString() <<
    " and back, is " << m_roundTripValue <<
    ", thus losing information.");
}


// create-tuple-class: definitions for XNumericConversionOutsideRange
/*AUTO_CTC*/ XNumericConversionOutsideRange::XNumericConversionOutsideRange(
/*AUTO_CTC*/   std::string const &sourceValue,
/*AUTO_CTC*/   TypeNameAndSize const &sourceType,
/*AUTO_CTC*/   TypeNameAndSize const &destType)
/*AUTO_CTC*/   : XNumericConversion(),
/*AUTO_CTC*/     IMEMBFP(sourceValue),
/*AUTO_CTC*/     IMEMBFP(sourceType),
/*AUTO_CTC*/     IMEMBFP(destType)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionOutsideRange::XNumericConversionOutsideRange(XNumericConversionOutsideRange const &obj) noexcept
/*AUTO_CTC*/   : XNumericConversion(obj),
/*AUTO_CTC*/     DMEMB(m_sourceValue),
/*AUTO_CTC*/     DMEMB(m_sourceType),
/*AUTO_CTC*/     DMEMB(m_destType)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionOutsideRange &XNumericConversionOutsideRange::operator=(XNumericConversionOutsideRange const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XNumericConversion::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_sourceValue);
/*AUTO_CTC*/     CMEMB(m_sourceType);
/*AUTO_CTC*/     CMEMB(m_destType);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XNumericConversionOutsideRange::getConflict() const
{
  return stringb(
    "convertNumber: Source value " << m_sourceValue <<
    " of type " << m_sourceType.toString() <<
    " cannot be represented with type " << m_destType.toString() <<
    ".");
}


// create-tuple-class: definitions for XNumericConversionFromAP
/*AUTO_CTC*/ XNumericConversionFromAP::XNumericConversionFromAP(
/*AUTO_CTC*/   std::string const &sourceTypeName,
/*AUTO_CTC*/   std::string const &sourceValue,
/*AUTO_CTC*/   bool destIsSigned,
/*AUTO_CTC*/   unsigned destSizeBytes)
/*AUTO_CTC*/   : XNumericConversion(),
/*AUTO_CTC*/     IMEMBFP(sourceTypeName),
/*AUTO_CTC*/     IMEMBFP(sourceValue),
/*AUTO_CTC*/     IMEMBFP(destIsSigned),
/*AUTO_CTC*/     IMEMBFP(destSizeBytes)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionFromAP::XNumericConversionFromAP(XNumericConversionFromAP const &obj) noexcept
/*AUTO_CTC*/   : XNumericConversion(obj),
/*AUTO_CTC*/     DMEMB(m_sourceTypeName),
/*AUTO_CTC*/     DMEMB(m_sourceValue),
/*AUTO_CTC*/     DMEMB(m_destIsSigned),
/*AUTO_CTC*/     DMEMB(m_destSizeBytes)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionFromAP &XNumericConversionFromAP::operator=(XNumericConversionFromAP const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XNumericConversion::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_sourceTypeName);
/*AUTO_CTC*/     CMEMB(m_sourceValue);
/*AUTO_CTC*/     CMEMB(m_destIsSigned);
/*AUTO_CTC*/     CMEMB(m_destSizeBytes);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XNumericConversionFromAP::getConflict() const
{
  return stringb(
    "Attempted to convert the " << m_sourceTypeName <<
    " value " << m_sourceValue << " to " <<
    (m_destIsSigned? "a signed " : "an unsigned ") <<
    (m_destSizeBytes*8) << "-bit integer type, but it does not fit.");
}


CLOSE_NAMESPACE(smbase)


// EOF
