// gdvsymbol.h
// `GDVSymbol`, which represents a symbol in a Generalized Data Value.

// This file is in the public domain.

#ifndef SMBASE_GDVSYMBOL_H
#define SMBASE_GDVSYMBOL_H

// this dir
#include "compare-util.h"              // DEFINE_FRIEND_NON_EQUALITY_RELATIONAL_OPERATORS
#include "sm-macros.h"                 // OPEN_NAMESPACE, DMEMB, CMEMB
#include "indexed-string-table.h"      // smbase::IndexedStringTable::Index

// libc++
#include <cstddef>                     // std::size_t
#include <iosfwd>                      // std::ostream [n]
#include <string>                      // std::string [n]
#include <string_view>                 // std::string_view [n]


OPEN_NAMESPACE(gdv)


// A symbol is the name of some entity or concept defined elsewhere.
// For example, the symbol `true` is a name that refers to Boolean
// truth, whereas the string "true" is simply a sequence of four
// letters.  That is, a symbol has primarily *extrinsic* meaning that
// depends on agreement between producer and consumer, whereas a string
// has primarily *intrinsic* meaning that is independent of the context.
class GDVSymbol {
public:      // types
  // Type of string table indices.
  using Index = smbase::IndexedStringTable::Index;

private:     // class data
  // Table of strings to which `m_symbolIndex` refers.  This table is
  // allocated the first time a symbol is created and lives for the
  // program lifetime.  It is a pointer rather than direct instance so I
  // can control the initialization order, and in particular, ensure
  // that a symbol can be created with program lifetime since I can
  // ensure the table gets built in time.
  static smbase::IndexedStringTable *s_stringTable;

  // Index of the string "null" in `s_stringTable`.  This allows the
  // default constructor to avoid looking it up.
  static Index s_nullSymbolIndex;

private:     // instance data
  // Index into `m_stringTable`.
  Index m_symbolIndex;

private:     // methods
  // Get the string table, making it if necessary.
  static smbase::IndexedStringTable *getStringTable();

public:      // methods
  ~GDVSymbol() = default;

  // Null symbol, i.e., a symbol whose name is "null".
  GDVSymbol()
    : m_symbolIndex(getNullSymbolIndex())
  {}

  // Convert string to corresponding symbol.  This makes a copy of the
  // string in `m_stringTable` if it is not already there.
  //
  // Requires `validSymbolName(s)`.
  explicit GDVSymbol(std::string_view const &s);

  // Create a `GDVSymbol` that stores `symbolIndex` directly.  The
  // caller must have obtained `symbolIndex` from a previous call to
  // `getSymbolIndex()`.
  //
  // I use an extra "tag" argument to ensure this is not called
  // unintentionally and does not conflict with the constructor that
  // takes a string view.
  enum DirectIndexTag { DirectIndex };
  explicit GDVSymbol(DirectIndexTag, Index symbolIndex);

  // `GDVSymbol` objects can be freely and cheaply copied.
  GDVSymbol(GDVSymbol const &obj)
    : DMEMB(m_symbolIndex) {}
  GDVSymbol &operator=(GDVSymbol const &obj)
    { CMEMB(m_symbolIndex); return *this; }

  // Comparison is by string contents, *not* index.
  friend int compare(GDVSymbol const &a, GDVSymbol const &b);
  DEFINE_FRIEND_NON_EQUALITY_RELATIONAL_OPERATORS(GDVSymbol)

  // Comparison for equality can be done more efficiently by directly
  // comparing indices rather than getting the string contents.
  bool operator==(GDVSymbol const &obj) const
    { return m_symbolIndex == obj.m_symbolIndex; }
  bool operator!=(GDVSymbol const &obj) const
    { return !operator==(obj); }

  // Get the number of bytes in this symbol's name.
  std::size_t size() const;

  // Get the numeric index for this symbol.  This can later be used to
  // construct a `GDVSymbol` without doing a lookup.  The actual index
  // value of a symbol can potentially change from run to run, as it
  // depends on the order in which symbols are seen, so should not
  // generally be exposed to the user.
  Index getSymbolIndex() const { return m_symbolIndex; }

  // Get the sequence of characters with the symbol name.  The returned
  // view is valid until the next symbol lookup is performed.
  std::string_view getSymbolName() const;

  // True if `name` conforms to the syntactic requirements of a symbol
  // name.  Specifically, it must match the regex
  /// "[a-zA-Z_][a-zA-Z_0-9]*".
  //
  // TODO: Remove this.
  static bool validSymbolName(std::string_view name);

  // Pass 'name' through the symbol table to get its index.  This is
  // safe to call in a global variable initializer because it takes care
  // of initializing prerequisites when necessary.
  //
  // Requires `validSymbolName(name)`.
  static Index lookupSymbolIndex(std::string_view name);

  // True if `i` is a valid index.
  static bool validIndex(Index i);

  // Given two indices obtained from `getSymbolIndex`, compare their
  // strings relationally.
  static int compareIndices(Index a, Index b);

  // Get the null symbol index.  This is equivalent to
  // `lookupSymbolIndex("null")` but may be faster.
  static Index getNullSymbolIndex();

  // Exchange names with 'obj'.
  void swap(GDVSymbol &obj);

  // Write the symbol to `os`.  Currently, this is just the name without
  // any form of quoting or escaping.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(std::ostream &os, GDVSymbol const &obj)
    { obj.write(os); return os; }

  // Return the string that `write` would write.
  std::string asString() const;
};

inline void swap(GDVSymbol &a, GDVSymbol &b)
{
  a.swap(b);
}


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVSYMBOL_H
