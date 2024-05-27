// vector-push-pop.h
// `VECTOR_PUSH_POP` macro to push a value and pop it on scope exit.

// This file is in the public domain.

#ifndef SMBASE_VECTOR_PUSH_POP_H
#define SMBASE_VECTOR_PUSH_POP_H

#include "dev-warning.h"               // DEV_WARNING
#include "sm-macros.h"                 // OPEN_NAMESPACE, NO_OBJECT_COPIES
#include "sm-pp-util.h"                // SM_PP_CAT

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
  VectorPushPop SM_PP_CAT(vpp_, __LINE__)(vec, value);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_VECTOR_PUSH_POP_H
