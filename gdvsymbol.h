// gdvsymbol.h
// GDVSymbol class.

#ifndef SMBASE_GDVSYMBOL_H
#define SMBASE_GDVSYMBOL_H

// this dir
#include "compare-util.h"              // DEFINE_FRIEND_RELATIONAL_OPERATORS
#include "strtable-fwd.h"              // StringTable [n]

// libc++
#include <cstddef>                     // std::size_t
#include <string>                      // std::string [n]


namespace gdv {


// A symbol is the name of some entity or concept defined elsewhere.
// For example, the symbol `true` is a name that refers to Boolean
// truth, whereas the string "true" is simply a sequence of four
// letters.
class GDVSymbol {
private:     // class data
  // Table of strings to which `m_symbolName` points.  This table is
  // allocated the first time a symbol is created and lives for the
  // program lifetime.  It is a pointer rather than direct instance so I
  // can control the initialization order, and in particular, ensure
  // that a symbol can be created with program lifetime since I can
  // ensure the table gets built in time.
  static StringTable *s_stringTable;

  // Pointer to the empty string in `s_stringTable`.  This allows the
  // default constructor to avoid looking up the empty string.
  static char const *s_emptySymbolName;

private:     // instance data
  // Pointer into `m_stringTable` providing the symbol name.  All
  // symbols that have the same name use the same pointer value.
  //
  // This is never `nullptr`, although it can point to an empty string.
  // The end of the name is marked by a NUL terminator character.
  //
  // This is what strtable.h calls a `StringRef` but I think that name
  // is a bit too collision-prone so I'm not using it here.
  //
  char const *m_symbolName;

private:     // methods
  // Get the string table, making it if necessary.
  static StringTable *getStringTable();

public:      // methods
  // Empty symbol, i.e., a symbol whose name is the empty string.
  GDVSymbol()
    : m_symbolName(getEmptySymbolName())
  {}

  // Convert string to corresponding symbol.  This makes a copy of the
  // string in `m_stringTable` if it is not already there.
  explicit GDVSymbol(std::string const &s);

  // Same, but using a NUL-terminated string pointer.  After this ctor,
  // `m_symbolName` will usually *not* equal `p`; it would only be equal
  // if `p` was itself some other symbol's `m_symbolName`.
  explicit GDVSymbol(char const *p);

  // Create a `GDVSymbol` that points to `symbolName` directly, without
  // looking it up in the symbol table.  The caller must have obtained
  // `symbolName` from a previous call to `getSymbolName()`.
  enum BypassSymbolLookupTag { BypassSymbolLookup };
  GDVSymbol(BypassSymbolLookupTag, char const *symbolName);

  // No deallocation is required since `m_symbolName` is not an owner
  // pointer, but we increment a counter in order to later check that
  // everything is balanced.
  ~GDVSymbol() {}

  // `GDVSymbol` objects can be freely and cheaply copied.
  GDVSymbol(GDVSymbol const &obj)
    : m_symbolName(obj.m_symbolName) {}
  GDVSymbol &operator=(GDVSymbol const &obj)
    { m_symbolName = obj.m_symbolName; return *this; }

  friend int compare(GDVSymbol const &a, GDVSymbol const &b);
  DEFINE_FRIEND_RELATIONAL_OPERATORS(GDVSymbol)

  // Get a pointer to a NUL-terminated string of characters with the
  // symbol name.
  char const *getSymbolName() const { return m_symbolName; }

  // Pass 'name' through the symbol table to get the unique
  // representative for the string it points to.  This is safe to call
  // in a global variable initializer because it takes care of
  // initializing prerequisites when necessary.
  static char const *lookupSymbolName(char const *name);

  // Get the empty symbol name.  This is equivalent to
  // `lookupSymbolName("")` but may be faster.
  static char const *getEmptySymbolName();

  // Exchange names with 'obj'.
  void swap(GDVSymbol &obj);
};

inline void swap(GDVSymbol &a, GDVSymbol &b)
{
  a.swap(b);
}


} // namespace gdv


#endif // SMBASE_GDVSYMBOL_H
