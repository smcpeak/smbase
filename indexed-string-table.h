// indexed-string-table.h
// Insert-only hashmap between strings (passed as `std::string_view`)
// and integers.  This is an updated version of `strtable.h` with added
// integer indices.

// This file is in the public domain.

#ifndef SMBASE_INDEXED_STRING_TABLE_H
#define SMBASE_INDEXED_STRING_TABLE_H

#include "indexed-string-table-fwd.h"  // fwds for this module

#include "hashtbl.h"                   // HashTable
#include "rack-allocator.h"            // smbase::RackAllocator
#include "sm-macros.h"                 // OPEN_NAMESPACE, NO_OBJECT_COPIES

#include <cstddef>                     // std::ptrdiff_t
#include <cstdint>                     // std::int32_t
#include <iosfwd>                      // std::ostream
#include <string_view>                 // std::string_view
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


/* Insert-only hashmap between strings and integers.

   For the purpose of this class, a "string" is a possibly-empty
   sequence of `char`.  Embedded NUL values are allowed.
*/
class IndexedStringTable {
  // For now at least.
  NO_OBJECT_COPIES(IndexedStringTable);

public:      // types
  // Size of a string.
  typedef std::int32_t Size;

  // Index into the table.  This is signed so I can use negative values
  // for downward iteration, invalid values, etc.
  typedef std::int32_t Index;

private:     // types
  // A record of a string.
  struct StoredString {
    // The index assigned to this string.
    //
    // If the index is non-negative, then this object is in the table,
    // and the string contents are stored contiguously after this
    // object.
    //
    // If the index is negative, then this object is actually a
    // `LookupString`, and its `m_string` pointer points to the string
    // data.
    Index m_index;

    // The length of the string in bytes (used for both cases).
    Size m_size;

  public:      // methods
    // Get this object's string.
    std::string_view getStringView() const;
  };

  // An object created temporarily during lookup operations.  A pointer
  // to this object, whose `m_index` is negative, serves as a key.
  struct LookupString : public StoredString {
    // Pointer to the start of the string to look up.
    char const *m_string;
  };

private:     // data
  // Allocator for storing `StoredString` objects (and the strings that
  // follow them).
  RackAllocator m_allocator;

  // Map from a string to its assigned index.
  HashTable m_stringToIndex;

  // Map from assigned index to the corresponding string.
  std::vector<StoredString*> m_indexToString;

private:     // methods
  // These are the functions we provide to `m_stringToIndex` to
  // customize its behavior.

  // Given a pointer to a `StoredString`, get a pointer to use as a hash
  // table key.  This just returns its argument.
  static void const *getKeyFromSS(void *dataSS);

  // Given a pointer to a `StoredString`, compute a hash of the stored
  // string.
  static unsigned hashSS(void const *dataSS);

  // Check if the two `StoredString` objects have equal string keys.
  static bool equalKeys(void const *dataSS1, void const *dataSS2);

public:      // methods
  ~IndexedStringTable();
  IndexedStringTable();

  // Number of strings stored.
  //
  // The return type is `Index` because this is conceptually part of
  // the space of indices (being max+1), not string sizes.
  Index size() const;

  // True if `i` can be passed to `get`.
  bool validIndex(Index i) const
    { return 0 <= i && i < size(); }

  // Add `str` to the table, returning its index.  If the string is
  // already present, this returns the previously-assigned index.  The
  // returned value is always in [0,size()-1].
  Index add(std::string_view str);

  // Get the string at `index`.
  //
  // Requires `validIndex(index)`.
  //
  // The returned view is invalidated by the next call to a non-const
  // method.
  std::string_view get(Index index) const;

  // Return </=/>0 if a</=/>b when compared as string *contents*.
  //
  // Note that checking for string equality is equivalent to checking
  // for index equality.  That is, `compareIndexedStrings(a,b)==0` iff
  // `a==b`.
  //
  // Requires `validIndex(a) && validIndex(b)`.
  int compareIndexedStrings(Index a, Index b) const;

  // Remove all entries.  This is the only way of removing entries.
  void clear();

  // Print some testing/performance stats to `os`.
  void printStats(std::ostream &os) const;

  // Assert invariants.
  void selfCheck() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_INDEXED_STRING_TABLE_H
