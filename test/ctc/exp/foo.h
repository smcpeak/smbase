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
  /*AUTO_CTC*/ explicit Foo(int x, float y, std::string const &z);
  /*AUTO_CTC*/ Foo(Foo const &obj);
  /*AUTO_CTC*/ Foo(Foo &&obj);
  /*AUTO_CTC*/ Foo &operator=(Foo const &obj);
  /*AUTO_CTC*/ Foo &operator=(Foo &&obj);
  /*AUTO_CTC*/ // For +compare:
  /*AUTO_CTC*/ friend int compare(Foo const &a, Foo const &b);
  /*AUTO_CTC*/ DEFINE_FRIEND_RELATIONAL_OPERATORS(Foo)
  /*AUTO_CTC*/ // For +write:
  /*AUTO_CTC*/ std::string toString() const;
  /*AUTO_CTC*/ void write(std::ostream &os);
  /*AUTO_CTC*/ friend std::ostream &operator<<(std::ostream &os, Foo const &obj);
};

#endif // FOO_H
