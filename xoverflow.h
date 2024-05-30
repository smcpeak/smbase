// xoverflow.h
// `XOverflow` exception class.

// This file is in the public domain.

#ifndef SMBASE_XOVERFLOW_H
#define SMBASE_XOVERFLOW_H

#include "xarithmetic.h"               // XArithmetic


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
  /*AUTO_CTC*/ XBinaryOpOverflow(XBinaryOpOverflow const &obj);
  /*AUTO_CTC*/ XBinaryOpOverflow(XBinaryOpOverflow &&obj);
  /*AUTO_CTC*/ XBinaryOpOverflow &operator=(XBinaryOpOverflow const &obj);
  /*AUTO_CTC*/ XBinaryOpOverflow &operator=(XBinaryOpOverflow &&obj);

  virtual std::string getConflict() const override;
};


// Conversion from one type to another loses information.
class XNumericConversion : public XOverflow {};


// Conversion from one type to another loses range.
class XNumericConversionLosesRange : public XNumericConversion {
public:      // data
  // Starting value.
  std::string m_sourceValue;

  // Value after conversion to destination type.
  std::string m_destValue;

  // Value after converting `m_destValue` back to the source type.  This
  // is different from `m_sourceValue`, hence the exception.
  std::string m_roundTripValue;

  // Size in bytes of the source type.
  unsigned m_sourceSizeBytes;

  // Size in bytes of the destination type.
  unsigned m_destSizeBytes;

public:      // methods
  // create-tuple-class: declarations for XNumericConversionLosesRange
  /*AUTO_CTC*/ explicit XNumericConversionLosesRange(std::string const &sourceValue, std::string const &destValue, std::string const &roundTripValue, unsigned sourceSizeBytes, unsigned destSizeBytes);
  /*AUTO_CTC*/ XNumericConversionLosesRange(XNumericConversionLosesRange const &obj);
  /*AUTO_CTC*/ XNumericConversionLosesRange(XNumericConversionLosesRange &&obj);
  /*AUTO_CTC*/ XNumericConversionLosesRange &operator=(XNumericConversionLosesRange const &obj);
  /*AUTO_CTC*/ XNumericConversionLosesRange &operator=(XNumericConversionLosesRange &&obj);

  virtual std::string getConflict() const override;
};


// Conversion from one type to another changes its sign.
class XNumericConversionChangesSign : public XNumericConversion {
public:      // data
  // TODO: It would be nice if I could add the common fields to the
  // base class and have create-tuple-class handle that properly.

  // Starting value.
  std::string m_sourceValue;

  // Value after conversion to destination type.
  std::string m_destValue;

public:      // methods
  // create-tuple-class: declarations for XNumericConversionChangesSign
  /*AUTO_CTC*/ explicit XNumericConversionChangesSign(std::string const &sourceValue, std::string const &destValue);
  /*AUTO_CTC*/ XNumericConversionChangesSign(XNumericConversionChangesSign const &obj);
  /*AUTO_CTC*/ XNumericConversionChangesSign(XNumericConversionChangesSign &&obj);
  /*AUTO_CTC*/ XNumericConversionChangesSign &operator=(XNumericConversionChangesSign const &obj);
  /*AUTO_CTC*/ XNumericConversionChangesSign &operator=(XNumericConversionChangesSign &&obj);

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
  /*AUTO_CTC*/ XNumericConversionFromAP(XNumericConversionFromAP const &obj);
  /*AUTO_CTC*/ XNumericConversionFromAP(XNumericConversionFromAP &&obj);
  /*AUTO_CTC*/ XNumericConversionFromAP &operator=(XNumericConversionFromAP const &obj);
  /*AUTO_CTC*/ XNumericConversionFromAP &operator=(XNumericConversionFromAP &&obj);

  virtual std::string getConflict() const override;
};


#endif // SMBASE_XOVERFLOW_H
