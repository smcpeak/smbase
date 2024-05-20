// sobjset.h            see license.txt for copyright and terms of use
// SObjSet, a non-owning set of objects implemented on top of HashTable.

#ifndef SMBASE_SOBJSET_H
#define SMBASE_SOBJSET_H

#include "hashtbl.h"    // HashTable

template <class T> class SObjSetIter;


// experiment: to try to support const-polymorphism,
// I expect T to either be Foo* or Foo const *

template <class T>
class SObjSet : private HashTable {
  friend class SObjSetIter<T>;

public:
  SObjSet(int initSize = HashTable::defaultSize)
    : HashTable(identityKeyFn, lcprngHashFn, pointerEqualKeyFn, initSize) {}

  // # of distinct elements in the set
  int size() const               { return HashTable::getNumEntries(); }

  // true if 'elt' is in the set
  bool contains(T elt) const     { return !!HashTable::get((void const*)elt); }

  // add 'elt' to the set; if it is already in, this has no effect
  void add(T elt)                { if (!contains(elt)) { HashTable::add((void const*)elt, (void*)elt); } }

  // remove 'elt' from the set; if it's not there, this has no effect
  void remove(T elt)             { if (contains(elt)) { HashTable::remove((void const*)elt); } }

  // remove all elements
  void empty()                   { HashTable::empty(); }

  // debug check which throws an exception if there's a problem
  void selfCheck()               { HashTable::selfCheck(); }
};


template <class T>
class SObjSetIter : private HashTableIter {
public:
  SObjSetIter(SObjSet<T> &table)
    : HashTableIter(table) {}    // I'm a friend, I can see it's a HashTable inside

  bool isDone() const          { return HashTableIter::isDone(); }
  void adv()                   { return HashTableIter::adv(); }
  T data() const               { return (T)HashTableIter::data(); }
};


#endif // SMBASE_SOBJSET_H
