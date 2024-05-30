// xarithmetic.h
// `XArithmetic`, an exception superclass for invalid arithmetic.

// This file is in the public domain.

#ifndef SMBASE_XARITHMETIC_H
#define SMBASE_XARITHMETIC_H

#include "exc.h"                       // XBase
#include "sm-macros.h"                 // OPEN_NAMESPACE


OPEN_NAMESPACE(smbase)


// Exception thrown when some sort of invalid numeric arithmetic
// operation is attempted.  Examples include overflow, conversion
// errors, and division by zero.
class XArithmetic : public XBase {};


// Attempt to divide by zero.
class XDivideByZero : public XArithmetic {
public:      // data
  // The numerator in the attempted division.
  std::string m_dividend;

public:      // methods
  // create-tuple-class: declarations for XDivideByZero
  /*AUTO_CTC*/ explicit XDivideByZero(std::string const &dividend);
  /*AUTO_CTC*/ XDivideByZero(XDivideByZero const &obj) noexcept;
  /*AUTO_CTC*/ XDivideByZero &operator=(XDivideByZero const &obj) noexcept;

  virtual std::string getConflict() const override;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_XARITHMETIC_H
