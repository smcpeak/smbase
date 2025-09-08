// ctc-test.h
// `CTCTest` class, which tests `create-tuple-class.py`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_CTC_TEST_H
#define SMBASE_CTC_TEST_H

#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "gdvalue-fwd.h"               // gdv::GDValue [n]
#include "gdvalue-parser-fwd.h"        // gdv::GDValueParser [n]

#include <iostream>                    // std::ostream
#include <string>                      // std::string


class CTCTest final {
public:
  int m_x;
  float m_y;
  std::string m_z;
  int m_w = 5;

  // Make sure the script does not choke on `using` declarations that
  // involve template specializations.
  using SomeTypeAlias = std::basic_string<char>;

public:
  // ---- create-tuple-class: declarations for CTCTest +compare +move +selfCheck +gdvWrite +gdvRead
  /*AUTO_CTC*/ explicit CTCTest(int x, float y, std::string const &z, int w = 5);
  /*AUTO_CTC*/ explicit CTCTest(int x, float y, std::string &&z, int w = 5);
  /*AUTO_CTC*/ CTCTest(CTCTest const &obj) noexcept;
  /*AUTO_CTC*/ CTCTest(CTCTest &&obj) noexcept;
  /*AUTO_CTC*/ void selfCheck() const;
  /*AUTO_CTC*/ CTCTest &operator=(CTCTest const &obj) noexcept;
  /*AUTO_CTC*/ CTCTest &operator=(CTCTest &&obj) noexcept;
  /*AUTO_CTC*/ // For +compare:
  /*AUTO_CTC*/ friend int compare(CTCTest const &a, CTCTest const &b);
  /*AUTO_CTC*/ DEFINE_FRIEND_RELATIONAL_OPERATORS(CTCTest)
  /*AUTO_CTC*/ // For +gdvWrite:
  /*AUTO_CTC*/ operator gdv::GDValue() const;
  /*AUTO_CTC*/ std::string toString() const;
  /*AUTO_CTC*/ void write(std::ostream &os) const;
  /*AUTO_CTC*/ friend std::ostream &operator<<(std::ostream &os, CTCTest const &obj);
  /*AUTO_CTC*/ // For +gdvRead:
  /*AUTO_CTC*/ explicit CTCTest(gdv::GDValueParser const &p);
};

#endif // SMBASE_CTC_TEST_H
