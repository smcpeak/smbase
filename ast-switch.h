// ast-switch.h
// `ASTSWITCH` macro, allowing one to "switch" on the type of an object.

// See license.txt for copyright and terms of use.

// See ast-switch.txt for explanation and discussion of the macros
// defined in this file.

#ifndef SMBASE_AST_SWITCH_H
#define SMBASE_AST_SWITCH_H


// ----------------------------- Downcasts -----------------------------
// The 'if' variants return nullptr. if the type isn't what's expected.
// The 'as' variants throw an exception in that case.
#define DECL_AST_DOWNCASTS(type, tag)            \
  type const *if##type##C() const;               \
  type *if##type()                               \
    { return const_cast<type*>(if##type##C()); } \
  type const *as##type##C() const;               \
  type *as##type()                               \
    { return const_cast<type*>(as##type##C()); } \
  bool is##type() const                          \
    { return kind() == tag; }


// Using this macro requires: #include "smbase/xassert.h"
#define DEFN_AST_DOWNCASTS(superclass, type, tag)\
  type const *superclass::if##type##C() const    \
  {                                              \
    if (kind() == tag) {                         \
      return (type const*)this;                  \
    }                                            \
    else {                                       \
      return NULL;                               \
    }                                            \
  }                                              \
                                                 \
  type const *superclass::as##type##C() const    \
  {                                              \
    xassert(kind() == tag);                      \
    return (type const*)this;                    \
  }


// -------------------------- Const typecase ---------------------------
#define ASTSWITCHC(supertype, nodeptr)           \
{                                                \
  supertype const *switch_nodeptr = (nodeptr);   \
  switch (switch_nodeptr->kind())

#define ASTCASEC(type, var)                           \
  case type::TYPE_TAG: {                              \
    type const *var = switch_nodeptr->as##type##C();

// The "1" versions mean "one argument", i.e., they do not bind a
// variable of the specified type.
#define ASTCASEC1(type)                               \
  case type::TYPE_TAG: {

#define ASTNEXTC(type, var)                           \
    break;                                            \
  } /* end previous case */                           \
  case type::TYPE_TAG: {                              \
    type const *var = switch_nodeptr->as##type##C();

#define ASTNEXTC1(type)                               \
    break;                                            \
  } /* end previous case */                           \
  case type::TYPE_TAG: {

// End a case, and add an empty `default` case.
#define ASTENDCASECD                                  \
    break;                                            \
  } /* end final case */                              \
  default: ;    /* silence warning */                 \
} /* end scope started before switch */

#define ASTDEFAULTC                                   \
    break;                                            \
  } /* end final case */                              \
  default: {

// End a case where an explicit default was present, or there is no need
// to add one (e.g., because it was exhaustive).
#define ASTENDCASEC                                   \
    break;                                            \
  } /* end final case */                              \
} /* end scope started before switch */


// ------------------------ Non-const typecase -------------------------
#define ASTSWITCH(supertype, nodeptr)            \
{                                                \
  supertype *switch_nodeptr = (nodeptr);         \
  switch (switch_nodeptr->kind())

#define ASTCASE(type, var)                            \
  case type::TYPE_TAG: {                              \
    type *var = switch_nodeptr->as##type();

#define ASTCASE1(type)                                \
  case type::TYPE_TAG: {

#define ASTNEXT(type, var)                            \
    break;                                            \
  } /* end previous case */                           \
  case type::TYPE_TAG: {                              \
    type *var = switch_nodeptr->as##type();

#define ASTNEXT1(type)                                \
    break;                                            \
  } /* end previous case */                           \
  case type::TYPE_TAG: {

// End-of-switch behavior is same as in const case.
#define ASTENDCASED ASTENDCASECD
#define ASTDEFAULT ASTDEFAULTC
#define ASTENDCASE ASTENDCASEC


// ---------------------- Const parallel typecase ----------------------
// `nodeptr1` and `nodeptr2` should already be known to have the same
// kind.
#define ASTSWITCH2C(supertype, nodeptr1, nodeptr2)      \
{                                                       \
  supertype const *switch_nodeptr1 = (nodeptr1);        \
  supertype const *switch_nodeptr2 = (nodeptr2);        \
  switch (switch_nodeptr1->kind())

#define ASTCASE2C(type, var1, var2)                     \
  case type::TYPE_TAG: {                                \
    type const *var1 = switch_nodeptr1->as##type##C();  \
    type const *var2 = switch_nodeptr2->as##type##C();

#define ASTCASE2C1(type)                                \
  case type::TYPE_TAG: {

#define ASTNEXT2C(type, var1, var2)                     \
    break;                                              \
  } /* end previous case */                             \
  case type::TYPE_TAG: {                                \
    type const *var1 = switch_nodeptr1->as##type##C();  \
    type const *var2 = switch_nodeptr2->as##type##C();

#define ASTNEXT2C1(type)                                \
    break;                                              \
  } /* end previous case */                             \
  case type::TYPE_TAG: {

// Same invocation syntax as ASTNEXT2C but without actually declaring
// the variables because they are unused (hence "U").  The "1" naming
// used above would clash with the "2" here that means something
// entirely different.
#define ASTNEXT2CU(type, var1, var2)                    \
    break;                                              \
  } /* end previous case */                             \
  case type::TYPE_TAG: {

#define ASTENDCASE2CD                                   \
    break;                                              \
  } /* end final case */                                \
  default: ;    /* silence warning */                   \
} /* end scope started before switch */

#define ASTDEFAULT2C                                    \
    break;                                              \
  } /* end final case */                                \
  default: {

#define ASTENDCASE2C                                    \
    break;                                              \
  } /* end final case */                                \
} /* end scope started before switch */


#endif // SMBASE_AST_SWITCH_H
