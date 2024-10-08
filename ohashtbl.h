// ohashtbl.h            see license.txt for copyright and terms of use
// OwnerHashTable, a hash table that owns the values.  Maps void* to T*.

// see hashtbl.h for more detail on the semantics of the member fns

#ifndef SMBASE_OHASHTBL_H
#define SMBASE_OHASHTBL_H

#include "exc.h"                       // GENERIC_CATCH_{BEGIN,END}
#include "hashtbl.h"                   // HashTable

template <class T> class OwnerHashTableIter;

template <class T>
class OwnerHashTable {
public:     // types
  friend class OwnerHashTableIter<T>;

  // see hashtbl.h
  typedef void const* (*GetKeyFn)(T *data);
  typedef unsigned (*HashFn)(void const *key);
  typedef bool (*EqualKeyFn)(void const *key1, void const *key2);

private:    // data
  // inner table that does the hash mapping
  HashTable table;

public:     // funcs
  OwnerHashTable(GetKeyFn gk, HashFn hf, EqualKeyFn ek,
                 int initSize = HashTable::defaultSize)
    : table((HashTable::GetKeyFn)gk, hf, ek, initSize) {}

  ~OwnerHashTable()
  {
    GENERIC_CATCH_BEGIN
    empty(1);
    GENERIC_CATCH_END
  }

  int getNumEntries() const               { return table.getNumEntries(); }
  T *get(void const *key) const           { return (T*)table.get(key); }
  void add(void const *key, T *value)     { table.add(key, value); }
  T *remove(void const *key)              { return (T*)table.remove(key); }
  void empty(int initSize = HashTable::defaultSize);
  void setEnableShrink(bool en)           { table.setEnableShrink(en); }
  void selfCheck() const                  { table.selfCheck(); }

  // this simply drops all the entries without deleting them; it is
  // useful when the objects have been taken out via iteration
  void disownAndForgetAll(int initSize = HashTable::defaultSize)
                                          { table.empty(initSize); }
};

template <class T>
void OwnerHashTable<T>::empty(int initSize)
{
  HashTableIter iter(table);
  for (; !iter.isDone(); iter.adv()) {
    delete (T*)iter.data();
  }
  table.empty(initSize);
}


template <class T>
class OwnerHashTableIter {
private:      // data
  HashTableIter iter;      // internal iterator

public:       // funcs
  OwnerHashTableIter(OwnerHashTable<T> &table)
    : iter(table.table) {}

  bool isDone() const      { return iter.isDone(); }
  void adv()               { iter.adv(); }
  T *data()                { return (T*)iter.data(); }
};

#endif // SMBASE_OHASHTBL_H
