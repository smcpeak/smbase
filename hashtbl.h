// hashtbl.h
// hash table mapping arbitrary keys to void*, where
// the stored pointers can be used to derive the key, and
// cannot be NULL

#ifndef HASHTBL_H
#define HASHTBL_H

#include "typ.h"     // STATICDEF

class HashTable {
private:    // types
  // constants
  enum {
    initialTableSize = 33,
  };

public:     // types
  // given a stored data pointer, retrieve the associated key
  typedef void const* (*GetKeyFn)(void *data);

  // given a key, retrieve the associated hash value;
  // this should be a 32-bit integer ready to be mod'd by the table size
  typedef unsigned (*HashFn)(void const *key);

  // compare two keys; this is needed so we can handle collisions
  // in the hash function; return true if they are equal
  typedef bool (*EqualKeyFn)(void const *key1, void const *key2);

private:    // data
  // maps
  GetKeyFn getKey;
  HashFn coreHashFn;
  EqualKeyFn equalKeys;

  // array of pointers to data, indexed by the hash value,
  // with collisions resolved by moving to adjacent entries;
  // some entries are NULL, meaning that hash value has no mapping
  void **hashTable;

  // number of slots in the hash table
  int tableSize;

  // number of mapped (non-NULL) entries
  int numEntries;

private:    // funcs
  // disallowed
  HashTable(HashTable&);
  void operator=(HashTable&);
  void operator==(HashTable&);

  // hash fn for the current table size; always in [0,tableSize-1]
  unsigned hashFunction(void const *key) const;

  // given a collision at 'index', return the next index to try
  int nextIndex(int index) const { return (index+1) % tableSize; }

  // resize the table, transferring all the entries to their
  // new positions
  void resizeTable(int newSize);

  // return the index of the entry corresponding to 'data' if it
  // is mapped, or a pointer to the entry that should be filled
  // with its mapping, if unmapped
  int getEntry(void const *key) const;

  // make a new table with the given size
  void makeTable(int size);

  // check a single entry for integrity
  void checkEntry(int entry) const;

public:     // funcs
  HashTable(GetKeyFn gk, HashFn hf, EqualKeyFn ek);
  ~HashTable();

  // return # of mapped entries
  int getNumEntries() const { return numEntries; }

  // if this hash value has a mapping, return it; otherwise,
  // return NULL
  void *get(void const *key) const;

  // add a mapping from 'key' to 'value'; there must not already
  // be a mapping for this key
  void add(void const *key, void *value);

  // remove the mapping for 'key' -- it must exist
  void remove(void const *key);

  // check the data structure's invariants, and throw an exception
  // if there is a problem
  void selfCheck() const;
};

#endif // HASHTBL_H
