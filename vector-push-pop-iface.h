// vector-push-pop-iface.h
// Interface decls for `vector-push-pop`.

// This file is in the public domain.

#ifndef SMBASE_VECTOR_PUSH_POP_IFACE_H
#define SMBASE_VECTOR_PUSH_POP_IFACE_H

#include "vector-push-pop-fwd.h"       // fwds for this module

#include "sm-macros.h"                 // OPEN_NAMESPACE, NO_OBJECT_COPIES
#include "sm-pp-util.h"                // SM_PP_CAT
#include "std-optional-fwd.h"          // std::optional [f]
#include "std-vector-fwd.h"            // stdfwd::vector [f]


OPEN_NAMESPACE(smbase)


// Push a value in the ctor, pop it in the dtor.
template <typename T>
class VectorPushPop {
  NO_OBJECT_COPIES(VectorPushPop);

public:      // data
  // The vector to manipulate.
  stdfwd::vector<T> &m_vec;

public:      // methods
  // Push a copy of `value`.
  inline VectorPushPop(stdfwd::vector<T> &vec, T const &value);

  // Push a moved `value`.
  inline VectorPushPop(stdfwd::vector<T> &vec, T &&value);

  // Pop.
  inline ~VectorPushPop();
};


// Deduce the value type from the vector type.
template <typename T, typename U>
VectorPushPop(stdfwd::vector<T> &, U &&) -> VectorPushPop<T>;


// Push `value` onto `vec` now, pop it off on scope exit.
//
// If this macro gets expanded, then it is necessary to include
// `vector-push-pop.h` (not just the `-iface` file).
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
  stdfwd::vector<T> &m_vec;

  // True if we should push and pop.
  //
  // It is permissible for a client to set this to false after
  // construction in order to inhibit the final pop even if the initial
  // push was done by this class.
  bool m_doPushPop;

public:      // methods
  // Push a copy of `*value` if it exists.
  inline VectorPushPopOpt(stdfwd::vector<T> &vec, std::optional<T> const &value);

  // Push a moved `*value` if it exists.
  inline VectorPushPopOpt(stdfwd::vector<T> &vec, std::optional<T> &&value);

  // Pop if `m_doPushPop`.
  inline ~VectorPushPopOpt();
};


// Deduce the value type from the vector type.
template <typename T, typename U>
VectorPushPopOpt(stdfwd::vector<T> &, U &&) -> VectorPushPopOpt<T>;


// If `valueOpt` has a value, push its value onto `vec` now, and pop it
// off on scope exit.  Otherwise do not push or pop.
#define VECTOR_PUSH_POP_OPT(vec, valueOpt) \
  smbase::VectorPushPopOpt SM_PP_CAT(vpp_, __LINE__)(vec, valueOpt);


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_VECTOR_PUSH_POP_IFACE_H
