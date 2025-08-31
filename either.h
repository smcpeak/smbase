// either.h
// `Either`, a `std::variant` with exactly two alternatives.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_EITHER_H
#define SMBASE_EITHER_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/xassert.h"            // xassert

#include <variant>                     // std::{get, variant}


OPEN_NAMESPACE(smbase)


// Holds either a `LEFT` or a `RIGHT`.
//
// For now, there is no assignment operator or `emplace` method, so this
// object never gets into the "valueless by exception" state.
//
template <typename LEFT, typename RIGHT>
class Either : public std::variant<LEFT, RIGHT> {
public:      // types
  using Base = std::variant<LEFT, RIGHT>;

public:
  using Base::Base;

  bool isLeft() const
    { return this->index() == 0; }
  bool isRight() const
    { return this->index() == 1; }

  LEFT const &leftC() const
    { xassert(isLeft()); return std::get<LEFT>(*this); }
  LEFT const &left() const
    { xassert(isLeft()); return std::get<LEFT>(*this); }
  LEFT &left()
    { xassert(isLeft()); return std::get<LEFT>(*this); }

  RIGHT const &rightC() const
    { xassert(isRight()); return std::get<RIGHT>(*this); }
  RIGHT const &right() const
    { xassert(isRight()); return std::get<RIGHT>(*this); }
  RIGHT &right()
    { xassert(isRight()); return std::get<RIGHT>(*this); }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_EITHER_H
