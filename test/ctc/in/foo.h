// foo.h
// `Foo` class.

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

#endif // FOO_H
