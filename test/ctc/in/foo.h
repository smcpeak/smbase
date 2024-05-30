// foo.h
// `Foo` class, for testing `create-tuple-class.py`.

#ifndef FOO_H
#define FOO_H

#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS

#include <iostream>                    // std::ostream
#include <string>                      // std::string

class Foo {
public:
  int m_x;
  float m_y;
  std::string m_z;

public:
  // ---- create-tuple-class: declarations for Foo +compare +write
};

class EmptyBase {};

class Bar : public EmptyBase {
public:
  int m_n;

public:
  // ---- create-tuple-class: declarations for Bar +compare +write
};

#endif // FOO_H
