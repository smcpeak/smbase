// vector-push-pop.h
// `VECTOR_PUSH_POP` macro to push a value and pop it on scope exit.

// This file is in the public domain.

// TODO: Make an interface version of this I can use in exc.h.

#ifndef SMBASE_VECTOR_PUSH_POP_H
#define SMBASE_VECTOR_PUSH_POP_H

#include "dev-warning.h"               // DEV_WARNING
#include "sm-macros.h"                 // OPEN_NAMESPACE, NO_OBJECT_COPIES
#include "sm-pp-util.h"                // SM_PP_CAT

#include <optional>                    // std::optional
#include <vector>                      // std::vector
#include <utility>                     // std::move


OPEN_NAMESPACE(smbase)


// Push a value in the ctor, pop it in the dtor.
template <typename T>
class VectorPushPop {
  NO_OBJECT_COPIES(VectorPushPop);

public:      // data
  // The vector to manipulate.
  std::vector<T> &m_vec;

public:      // methods
  VectorPushPop(std::vector<T> &vec, T const &value)
    : m_vec(vec)
  {
    m_vec.push_back(value);
  }

  VectorPushPop(std::vector<T> &vec, T &&value)
    : m_vec(vec)
  {
    m_vec.push_back(std::move(value));
  }

  ~VectorPushPop()
  {
    if (m_vec.empty()) {
      DEV_WARNING("vector to pop is empty");
    }
    else {
      m_vec.pop_back();
    }
  }
};


// Push `value` onto `vec` now, pop it off on scope exit.
//
// This macro synthesizes a variable name based on the line number so
// that multiple uses in the same function do not collide.
#define VECTOR_PUSH_POP(vec, value) \
  smbase::VectorPushPop SM_PP_CAT(vpp_, __LINE__)(vec, value);


// Optionally push and pop using an optional value.
template <typename T>
class VectorPushPopOpt {
  NO_OBJECT_COPIES(VectorPushPopOpt);

public:      // data
  // The vector to manipulate.
  std::vector<T> &m_vec;

  // True if we should push and pop.
  //
  // It is permissible for a client to set this to false after
  // construction in order to inhibit the final pop even if the initial
  // push was done by this class.
  bool m_doPushPop;

public:      // methods
  VectorPushPopOpt(std::vector<T> &vec, std::optional<T> const &value)
    : m_vec(vec),
      m_doPushPop(value.has_value())
  {
    if (m_doPushPop) {
      m_vec.push_back(*value);
    }
  }

  VectorPushPopOpt(std::vector<T> &vec, std::optional<T> &&value)
    : m_vec(vec),
      m_doPushPop(value.has_value())
  {
    if (m_doPushPop) {
      m_vec.push_back(std::move(*value));
    }
  }

  ~VectorPushPopOpt()
  {
    if (m_doPushPop) {
      if (m_vec.empty()) {
        DEV_WARNING("vector to pop is empty");
      }
      else {
        m_vec.pop_back();
      }
    }
  }
};


// If `valueOpt` has a value, push its value onto `vec` now, and pop it
// off on scope exit.  Otherwise do not push or pop.
#define VECTOR_PUSH_POP_OPT(vec, valueOpt) \
  smbase::VectorPushPopOpt SM_PP_CAT(vpp_, __LINE__)(vec, valueOpt);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_VECTOR_PUSH_POP_H
