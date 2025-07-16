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
  // ---- create-tuple-class: declarations for Foo +compare +write +move
  /*AUTO_CTC*/ explicit Foo(int x, float y, std::string const &z);
  /*AUTO_CTC*/ explicit Foo(int x, float y, std::string &&z);
  /*AUTO_CTC*/ Foo(Foo const &obj) noexcept;
  /*AUTO_CTC*/ Foo(Foo &&obj) noexcept;
  /*AUTO_CTC*/ Foo &operator=(Foo const &obj) noexcept;
  /*AUTO_CTC*/ Foo &operator=(Foo &&obj) noexcept;
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
  /*AUTO_CTC*/ Bar(Bar const &obj) noexcept;
  /*AUTO_CTC*/ Bar &operator=(Bar const &obj) noexcept;
  /*AUTO_CTC*/ // For +compare:
  /*AUTO_CTC*/ friend int compare(Bar const &a, Bar const &b);
  /*AUTO_CTC*/ DEFINE_FRIEND_RELATIONAL_OPERATORS(Bar)
  /*AUTO_CTC*/ // For +write:
  /*AUTO_CTC*/ std::string toString() const;
  /*AUTO_CTC*/ void write(std::ostream &os) const;
  /*AUTO_CTC*/ friend std::ostream &operator<<(std::ostream &os, Bar const &obj);
};

struct Baz {
  int *m_p;

  // ---- create-tuple-class: declarations for Baz
  /*AUTO_CTC*/ explicit Baz(int *p);
  /*AUTO_CTC*/ Baz(Baz const &obj) noexcept;
  /*AUTO_CTC*/ Baz &operator=(Baz const &obj) noexcept;
};

#endif // FOO_H
