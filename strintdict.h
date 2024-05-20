// strintdict.h            see license.txt for copyright and terms of use
// Dictionary of intptr_t (integers that fit into void*), indexed by string
// (case-sensitive).

// NOTE: automatically generated from xstrobjdict.h -- do not edit directly

#ifndef STRINTDICT_H
#define STRINTDICT_H

#include "strutil.h"                   // qsortStringArray
#include "svdict.h"                    // StringVoidDict

// since the dictionary does not own the pointed-to objects,
// it has the same constness model as StringVoidDict, namely
// that const means the *mapping* is constant but not the
// pointed-to objects


class StringIntDict {
public:     // types
  // 'foreach' iterator functions
  typedef bool (*ForeachFn)(OldSmbaseString const &key, intptr_t value, void *extra);

  // external iterator
  class Iter {
  private:
    StringVoidDict::Iter iter;

  public:
    Iter(StringIntDict &dict) : iter(dict.dict) {}
    Iter(Iter const &obj) : DMEMB(iter) {}
    Iter& operator= (Iter const &obj) { CMEMB(iter); return *this; }

    bool isDone() const { return iter.isDone(); }
    Iter& next() { iter.next(); return *this; }

    OldSmbaseString const &key() const { return iter.key(); }
    intptr_t &value() const { return (intptr_t &)iter.value(); }

    int private_getCurrent() const { return iter.private_getCurrent(); }
  };
  friend class Iter;

  class IterC {
  private:
    StringVoidDict::IterC iter;

  public:
    IterC(StringIntDict const &dict) : iter(dict.dict) {}
    IterC(IterC const &obj) : DMEMB(iter) {}
    IterC& operator= (IterC const &obj) { CMEMB(iter); return *this; }

    bool isDone() const { return iter.isDone(); }
    IterC& next() { iter.next(); return *this; }

    OldSmbaseString const &key() const { return iter.key(); }
    intptr_t value() const { return (intptr_t)iter.value(); }

    int private_getCurrent() const { return iter.private_getCurrent(); }
  };
  friend class IterC;

  class SortedKeyIter {
  private:     // data
    // underlying iterator state
    StringIntDict const &map;
    int keyIndex;
    // dsw: I think it is a mistake to use getNumEntries() repeatedly
    // instead of caching the value that it was at the time the
    // iterator was constructed.  While we are still not thread-safe
    // in the sense that an entry could be added or deleted, we could
    // catch that, but we still do not want to separate the array of
    // sortedKeys from its length as if these get out of synch this is
    // an impossible bug to catch and a very low-level error.  Note
    // that we still have a bit of a race condidition that numEntries
    // is initialized before we iterate over the keys, but we are
    // likely to catch that later.
    int const numEntries;
    char const **sortedKeys;    // array of strings

  public:      // fucs
    SortedKeyIter(StringIntDict const &map0)
      : map(map0)
      , keyIndex(0)
      , numEntries(map.size())
      , sortedKeys(new char const *[numEntries])
    {
      int i = 0;
      // delegate to the other Iter class
      for(IterC iter(map); !iter.isDone(); iter.next()) {
//          xassert(i<numEntries);
        sortedKeys[i++] = iter.key().c_str();
      }
      xassert(numEntries == i);
      ::qsortStringArray(sortedKeys, numEntries);
    }
    ~SortedKeyIter() {
      delete [] sortedKeys;
    }

    bool isDone() const   { return keyIndex == numEntries; }
    SortedKeyIter &next() { ++keyIndex; return *this; }

    // return information about the currently-referenced table entry

    // dsw: I already have a char const* so I think it is a mistake to
    // wrap a string around it
    char const *key() const {
      char const *key = sortedKeys[keyIndex];
//        xassert(map.isMapped(key));
      return key;
    }
    intptr_t value() const {
      return (intptr_t )map.queryfC(key());
    }
  };

private:    // data
  // underlying dictionary functionality
  StringVoidDict dict;

public:     // funcs
  StringIntDict() : dict() {}
  ~StringIntDict() {}

public:    // funcs
  StringIntDict(StringIntDict const &obj) : dict(obj.dict) {}

  // comparison and assignment use *pointer* comparison/assignment

  StringIntDict& operator= (StringIntDict const &obj)    { dict = obj.dict; return *this; }

  bool operator== (StringIntDict const &obj) const        { return dict == obj.dict; }
  NOTEQUAL_OPERATOR(StringIntDict)

  // due to similarity with StringVoidDict, see svdict.h for
  // details on these functions' interfaces

public:
  // ------- selectors ---------
  int size() const                                     { return dict.size(); }

  bool isEmpty() const                                 { return dict.isEmpty(); }
  bool isNotEmpty() const                              { return !isEmpty(); }

  bool query(char const *key, intptr_t &value) const         { return dict.query(key, (void*&)value); }
  intptr_t queryf(char const *key) const                     { return (intptr_t)dict.queryf(key); }
  intptr_t queryif(char const *key) const                    { return (intptr_t)dict.queryif(key); }

  // parallel functions for API consistency
  bool queryC(char const *key, intptr_t &value) const { return query(key, value); }
  intptr_t queryfC(char const *key) const { return queryf(key); }

  bool isMapped(char const *key) const                 { return dict.isMapped(key); }

  // -------- mutators -----------
  void add(char const *key, intptr_t value)                  { dict.add(key, (void*)value); }
  intptr_t /*owner*/ remove(char const *key)                { return (intptr_t)dict.remove(key); }
  intptr_t modify(char const *key, intptr_t newValue)              { return (intptr_t)dict.modify(key, (void*)newValue); }

  void empty()                                         { dict.empty(); }

  // -------- parallel interface for 'rostring' --------
  bool query(rostring key, intptr_t &value) const { return query(key.c_str(), value); }
  intptr_t queryf(rostring key) const             { return queryf(key.c_str()); }
  intptr_t queryif(rostring key) const            { return queryif(key.c_str()); }
  bool isMapped(rostring key) const         { return isMapped(key.c_str()); }
  void add(rostring key, intptr_t value)          { dict.add(key, (void*)value); }
  intptr_t modify(rostring key, intptr_t newValue)      { return modify(key.c_str(), newValue); }
  intptr_t remove(rostring key)                   { return remove(key.c_str()); }

  // --------- iters -------------
  void foreach(ForeachFn func, void *extra=NULL) const
  {
    // GCC -Wextra complains about this cast because the 'void*'
    // parameter in the destination type does not match the 'intptr_t'
    // parameter in the source type.  It's probably right that this is
    // technically undefined behavior, but fixing that is a significant
    // retrofit to old code that works in practice on every
    // implementation I'm aware of, so for now I'm choosing not to fix
    // it.  Instead, I use -Wno-cast-function-type.
    dict.foreach((StringVoidDict::ForeachFn)func, extra);
  }

  // ------------ misc --------------
  // debugging
  int private_getTopAddr() const { return dict.private_getTopAddr(); }
};


#endif // STRINTDICT_H
