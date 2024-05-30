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
  /*AUTO_CTC*/ void write(std::ostream &os) const;
  /*AUTO_CTC*/ friend std::ostream &operator<<(std::ostream &os, Foo const &obj);
};

class EmptyBase {};

class Bar : public EmptyBase {
public:
  int m_n;

public:
  // ---- create-tuple-class: declarations for Bar +compare +write
  /*AUTO_CTC*/ explicit Bar(int n);
  /*AUTO_CTC*/ Bar(Bar const &obj);
  /*AUTO_CTC*/ Bar(Bar &&obj);
  /*AUTO_CTC*/ Bar &operator=(Bar const &obj);
  /*AUTO_CTC*/ Bar &operator=(Bar &&obj);
  /*AUTO_CTC*/ // For +compare:
  /*AUTO_CTC*/ friend int compare(Bar const &a, Bar const &b);
  /*AUTO_CTC*/ DEFINE_FRIEND_RELATIONAL_OPERATORS(Bar)
  /*AUTO_CTC*/ // For +write:
  /*AUTO_CTC*/ std::string toString() const;
  /*AUTO_CTC*/ void write(std::ostream &os) const;
  /*AUTO_CTC*/ friend std::ostream &operator<<(std::ostream &os, Bar const &obj);
};

#endif // FOO_H
