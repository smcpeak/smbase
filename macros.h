// macros.h
// grab-bag of useful macros, stashed here to avoid mucking up
//   other modules with more focus; there's no clear rhyme or
//   reason for why some stuff is here and some in typ.h
// (no configuration stuff here!)

#ifndef __MACROS_H
#define __MACROS_H

#include "typ.h"        // bool

// complement of ==
#define NOTEQUAL_OPERATOR(T)             \
  bool operator != (T const &obj) const  \
    { return !operator==(obj); }

// toss this into a class that already has == and < defined, to
// round out the set of relational operators
#define RELATIONAL_OPERATORS(T)                    \
  NOTEQUAL_OPERATOR(T)                             \
  bool operator <= (T const &obj) const            \
    { return !obj.operator<(*this); }              \
  bool operator > (T const &obj) const             \
    { return obj.operator<(*this); }               \
  bool operator >= (T const &obj) const            \
    { return !operator<(obj); }


// member copy in constructor initializer list
#define DMEMB(var) var(obj.var)

// member copy in operator =
#define CMEMB(var) var = obj.var

// member comparison in operator ==
#define EMEMB(var) var == obj.var


// standard insert operator
// (note that you can put 'virtual' in front of the macro call if desired)
#define INSERT_OSTREAM(T)                                \
  void insertOstream(ostream &os) const;                 \
  friend ostream& operator<< (ostream &os, T const &obj) \
    { obj.insertOstream(os); return os; }


// usual declarations for a data object (as opposed to control object)
#define DATA_OBJ_DECL(T)                \
  T();                                  \
  T(T const &obj);                      \
  ~T();                                 \
  T& operator= (T const &obj);          \
  bool operator== (T const &obj) const; \
  NOTEQUAL_OPERATOR(T)                  \
  INSERTOSTREAM(T)


// copy this to the .cc file for implementation of DATA_OBJ_DECL
#if 0
T::T()
{}

T::T(T const &obj)
  : DMEMB(),
    DMEMB(),
    DMEMB()
{}

T::~T()
{}

T& T::operator= (T const &obj)
{
  if (this != &obj) {
    CMEMB();
  }
  return *this;
}

bool T::operator== (T const &obj) const
{
  return
    EMEMB() &&
    EMEMB();
}

void T::insertOstream(ostream &os) const
{}
#endif // 0


// assert something at compile time (must use this inside a function);
// works because compilers won't let us declare negative-length arrays
// (the expression below works with egcs-1.1.2)
#define STATIC_ASSERT(cond) \
  { (void)((int (*)(char failed_static_assertion[(cond)?1:-1]))0); }

  
// for silencing variable-not-used warnings
template <class T>
inline void pretendUsedFn(T const &) {}
#define PRETEND_USED(arg) pretendUsedFn(arg) /* user ; */


// appended to function declarations to indicate they do not
// return control to their caller; e.g.:
//   void exit(int code) NORETURN;
#ifdef __GNUC__
  #define NORETURN __attribute__((noreturn))
#else             
  // just let the warnings roll if we can't suppress them
  #define NORETURN
#endif


// these two are a common idiom in my code for typesafe casts;
// they are essentially a roll-your-own RTTI
#define CAST_MEMBER_FN(destType)                                                \
  destType const &as##destType##C() const;                                      \
  destType &as##destType() { return const_cast<destType&>(as##destType##C()); }

#define CAST_MEMBER_IMPL(inClass, destType)         \
  destType const &inClass::as##destType##C() const  \
  {                                                 \
    xassert(is##destType());                        \
    return (destType const&)(*this);                \
  }

#endif // __MACROS_H
