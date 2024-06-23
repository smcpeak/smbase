// xoverflow.cc
// Code for `xoverflow.h`.

#include "xoverflow.h"                 // this module

#include "sm-macros.h"                 // DMEMB, CMEMB
#include "string-util.h"               // doubleQuote
#include "stringb.h"                   // stringb


OPEN_NAMESPACE(smbase)


// create-tuple-class: definitions for XBinaryOpOverflow
/*AUTO_CTC*/ XBinaryOpOverflow::XBinaryOpOverflow(
/*AUTO_CTC*/   std::string const &type,
/*AUTO_CTC*/   std::string const &lhs,
/*AUTO_CTC*/   std::string const &rhs,
/*AUTO_CTC*/   std::string const &op)
/*AUTO_CTC*/   : XOverflow(),
/*AUTO_CTC*/     m_type(type),
/*AUTO_CTC*/     m_lhs(lhs),
/*AUTO_CTC*/     m_rhs(rhs),
/*AUTO_CTC*/     m_op(op)
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
    "Arithmetic overflow of type " << doubleQuote(m_type) << ": " <<
    m_lhs << ' ' << m_op << ' ' << m_rhs << " would overflow.");
}


// create-tuple-class: definitions for XNumericConversionLosesRange
/*AUTO_CTC*/ XNumericConversionLosesRange::XNumericConversionLosesRange(
/*AUTO_CTC*/   std::string const &sourceValue,
/*AUTO_CTC*/   std::string const &destValue,
/*AUTO_CTC*/   std::string const &roundTripValue,
/*AUTO_CTC*/   unsigned sourceSizeBytes,
/*AUTO_CTC*/   unsigned destSizeBytes)
/*AUTO_CTC*/   : XNumericConversion(),
/*AUTO_CTC*/     m_sourceValue(sourceValue),
/*AUTO_CTC*/     m_destValue(destValue),
/*AUTO_CTC*/     m_roundTripValue(roundTripValue),
/*AUTO_CTC*/     m_sourceSizeBytes(sourceSizeBytes),
/*AUTO_CTC*/     m_destSizeBytes(destSizeBytes)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionLosesRange::XNumericConversionLosesRange(XNumericConversionLosesRange const &obj) noexcept
/*AUTO_CTC*/   : XNumericConversion(obj),
/*AUTO_CTC*/     DMEMB(m_sourceValue),
/*AUTO_CTC*/     DMEMB(m_destValue),
/*AUTO_CTC*/     DMEMB(m_roundTripValue),
/*AUTO_CTC*/     DMEMB(m_sourceSizeBytes),
/*AUTO_CTC*/     DMEMB(m_destSizeBytes)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionLosesRange &XNumericConversionLosesRange::operator=(XNumericConversionLosesRange const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XNumericConversion::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_sourceValue);
/*AUTO_CTC*/     CMEMB(m_destValue);
/*AUTO_CTC*/     CMEMB(m_roundTripValue);
/*AUTO_CTC*/     CMEMB(m_sourceSizeBytes);
/*AUTO_CTC*/     CMEMB(m_destSizeBytes);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XNumericConversionLosesRange::getConflict() const
{
  return stringb(
    "Source value " << m_sourceValue <<
    " converts to destination value " << m_destValue <<
    " and back to different value " << m_roundTripValue <<
    " (ss=" << m_sourceSizeBytes << " ds=" << m_destSizeBytes << ").");
}


// create-tuple-class: definitions for XNumericConversionChangesSign
/*AUTO_CTC*/ XNumericConversionChangesSign::XNumericConversionChangesSign(
/*AUTO_CTC*/   std::string const &sourceValue,
/*AUTO_CTC*/   std::string const &destValue)
/*AUTO_CTC*/   : XNumericConversion(),
/*AUTO_CTC*/     m_sourceValue(sourceValue),
/*AUTO_CTC*/     m_destValue(destValue)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionChangesSign::XNumericConversionChangesSign(XNumericConversionChangesSign const &obj) noexcept
/*AUTO_CTC*/   : XNumericConversion(obj),
/*AUTO_CTC*/     DMEMB(m_sourceValue),
/*AUTO_CTC*/     DMEMB(m_destValue)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ XNumericConversionChangesSign &XNumericConversionChangesSign::operator=(XNumericConversionChangesSign const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     XNumericConversion::operator=(obj);
/*AUTO_CTC*/     CMEMB(m_sourceValue);
/*AUTO_CTC*/     CMEMB(m_destValue);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


std::string XNumericConversionChangesSign::getConflict() const
{
  return stringb(
    "convertNumber: Source value " << m_sourceValue <<
    " and destination value " << m_destValue <<
    " have different signs.");
}


// create-tuple-class: definitions for XNumericConversionFromAP
/*AUTO_CTC*/ XNumericConversionFromAP::XNumericConversionFromAP(
/*AUTO_CTC*/   std::string const &sourceTypeName,
/*AUTO_CTC*/   std::string const &sourceValue,
/*AUTO_CTC*/   bool destIsSigned,
/*AUTO_CTC*/   unsigned destSizeBytes)
/*AUTO_CTC*/   : XNumericConversion(),
/*AUTO_CTC*/     m_sourceTypeName(sourceTypeName),
/*AUTO_CTC*/     m_sourceValue(sourceValue),
/*AUTO_CTC*/     m_destIsSigned(destIsSigned),
/*AUTO_CTC*/     m_destSizeBytes(destSizeBytes)
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
