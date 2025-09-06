// foo.h
// `Foo` class, for testing `create-tuple-class.py`.

#ifndef FOO_H
#define FOO_H

#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "gdvalue-fwd.h"               // gdv::GDValue [n]
#include "gdvalue-parser-fwd.h"        // gdv::GDValueParser [n]

#include <iostream>                    // std::ostream
#include <string>                      // std::string

class Foo {
public:
  int m_x;
  float m_y;
  std::string m_z;

public:
  // ---- create-tuple-class: declarations for Foo +compare +write +move +selfCheck -writeDefn +gdvWrite +gdvRead
};

class EmptyBase {};

class Bar : public EmptyBase {
public:
  int m_n;

public:
  // ---- create-tuple-class: declarations for Bar +compare +write
};

struct Baz final {
  int *m_p;

  // ---- create-tuple-class: declarations for Baz
};

#endif // FOO_H
