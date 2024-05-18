// gdvsymbol.h
// GDVSymbol class.

#ifndef SMBASE_GDVSYMBOL_H
#define SMBASE_GDVSYMBOL_H

// this dir
#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS

// libc++
#include <string>                      // std::string


namespace gdv {


// This is planned to be a pointer into a symbol table, but for now is
// just a normal string.
class GDVSymbol {
public:      // data
  std::string m_symbolName;

public:      // methods
  GDVSymbol();
  explicit GDVSymbol(std::string const &s);
  explicit GDVSymbol(char const *p);

  ~GDVSymbol();

  friend int compare(GDVSymbol const &a, GDVSymbol const &b);
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDVSymbol)
};


} // namespace gdv


#endif // SMBASE_GDVSYMBOL_H
