// xoverflow.h
// `XOverflow` exception class.

// This file is in the public domain.

#ifndef SMBASE_XOVERFLOW_H
#define SMBASE_XOVERFLOW_H

#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "xarithmetic.h"               // XArithmetic


OPEN_NAMESPACE(smbase)


// Exception thrown when there would be an arithmetic overflow due to
// limited range of the representation type.
class XOverflow : public XArithmetic {};


// Overflow due to a binary arithmetic operation.
class XBinaryOpOverflow : public XOverflow {
public:      // data
  // The name of the type upon which the operation was performed.  This
  // is the type of both operands and of the intended result.
  //
  // TODO: Currently this is `typeid.name()`, which is cryptic.
  std::string m_type;

  // The involved operand values, as digit strings.
  std::string m_lhs;
  std::string m_rhs;

  // The operation, e.g., "+".
  std::string m_op;

public:      // methods
  // create-tuple-class: declarations for XBinaryOpOverflow
  /*AUTO_CTC*/ explicit XBinaryOpOverflow(std::string const &type, std::string const &lhs, std::string const &rhs, std::string const &op);
  /*AUTO_CTC*/ XBinaryOpOverflow(XBinaryOpOverflow const &obj) noexcept;
  /*AUTO_CTC*/ XBinaryOpOverflow &operator=(XBinaryOpOverflow const &obj) noexcept;

  virtual std::string getConflict() const override;
};


// Conversion from one type to another fails.
class XNumericConversion : public XOverflow {};


// Conversion from one type to another loses information, in that a
// conversion back to the original type yields a different value.  The
// check that throws this tolerates cases where the source and
// destination values are different, but the conversion to the source
// type yields the original value, so this exception is *not* thrown for
// a case like that, such as:
//
//   (signed char)-1 -> (unsigned char)255 -> (signed char)-1
//
class XNumericConversionLosesInformation : public XNumericConversion {
public:      // data
  // Starting value.
  std::string m_sourceValue;

  // Value after conversion to destination type.
  std::string m_destValue;

  // Value after converting `m_destValue` back to the source type.  This
  // is different from `m_sourceValue`, hence the exception.
  std::string m_roundTripValue;

  // Types of source and destination.
  std::string m_sourceType;
  std::string m_destType;

public:      // methods
  // create-tuple-class: declarations for XNumericConversionLosesInformation
  /*AUTO_CTC*/ explicit XNumericConversionLosesInformation(std::string const &sourceValue, std::string const &destValue, std::string const &roundTripValue, std::string const &sourceType, std::string const &destType);
  /*AUTO_CTC*/ XNumericConversionLosesInformation(XNumericConversionLosesInformation const &obj) noexcept;
  /*AUTO_CTC*/ XNumericConversionLosesInformation &operator=(XNumericConversionLosesInformation const &obj) noexcept;

  virtual std::string getConflict() const override;
};


// Conversion from one type to another is not possible because the
// source value is outside the range of the destination value.  The code
// that throws this exception wants the source and destination values to
// be exactly the same, so it *will* flag a case like converting
// `(signed char)-1` to `unsigned char`.
class XNumericConversionOutsideRange : public XNumericConversion {
public:      // data
  // TODO: It would be nice if I could add the common fields to the
  // base class and have create-tuple-class handle that properly.

  // Starting value.
  std::string m_sourceValue;

  // Original type of the source value.
  std::string m_sourceType;

  // Type to which conversion was attempted.
  std::string m_destType;

public:      // methods
  // create-tuple-class: declarations for XNumericConversionOutsideRange
  /*AUTO_CTC*/ explicit XNumericConversionOutsideRange(std::string const &sourceValue, std::string const &sourceType, std::string const &destType);
  /*AUTO_CTC*/ XNumericConversionOutsideRange(XNumericConversionOutsideRange const &obj) noexcept;
  /*AUTO_CTC*/ XNumericConversionOutsideRange &operator=(XNumericConversionOutsideRange const &obj) noexcept;

  virtual std::string getConflict() const override;
};


// Conversion from an arbitrary-precision integer to a fixed-size
// integer fails because the destination type has insufficient range.
class XNumericConversionFromAP : public XNumericConversion {
public:      // data
  // Name of the AP class we started with.
  std::string m_sourceTypeName;

  // Starting numeric value.
  std::string m_sourceValue;

  // True if the destination type is signed, false if unsigned.
  bool m_destIsSigned;

  // Size in bytes of the destination type.
  unsigned m_destSizeBytes;

public:      // methods
  // create-tuple-class: declarations for XNumericConversionFromAP
  /*AUTO_CTC*/ explicit XNumericConversionFromAP(std::string const &sourceTypeName, std::string const &sourceValue, bool destIsSigned, unsigned destSizeBytes);
  /*AUTO_CTC*/ XNumericConversionFromAP(XNumericConversionFromAP const &obj) noexcept;
  /*AUTO_CTC*/ XNumericConversionFromAP &operator=(XNumericConversionFromAP const &obj) noexcept;

  virtual std::string getConflict() const override;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_XOVERFLOW_H
