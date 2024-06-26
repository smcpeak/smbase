// strsobjdict.h            see license.txt for copyright and terms of use
// StringSObjDict, a case-sensitive map from strings to object pointers.
// The dictionary does *not* own the referred-to objects.

// NOTE: automatically generated from xstrobjdict.h -- do not edit directly

#ifndef STRSOBJLIST_H
#define STRSOBJLIST_H

#include "strutil.h"                   // qsortStringArray
#include "svdict.h"                    // StringVoidDict

// since the dictionary does not own the pointed-to objects,
// it has the same constness model as StringVoidDict, namely
// that const means the *mapping* is constant but not the
// pointed-to objects

template <class T>
class StringSObjDict {
public:     // types
  // 'foreach' iterator functions
  typedef bool (*ForeachFn)(string const &key, T * value, void *extra);

  // external iterator
  class Iter {
  private:
    StringVoidDict::Iter iter;

  public:
    Iter(StringSObjDict &dict) : iter(dict.dict) {}
    Iter(Iter const &obj) : DMEMB(iter) {}
    Iter& operator= (Iter const &obj) { CMEMB(iter); return *this; }

    bool isDone() const { return iter.isDone(); }
    Iter& next() { iter.next(); return *this; }

    string key() const { return iter.key(); }
    T * &value() const { return (T * &)iter.value(); }

    int private_getCurrent() const { return iter.private_getCurrent(); }
  };
  friend class Iter;

  class IterC {
  private:
    StringVoidDict::IterC iter;

  public:
    IterC(StringSObjDict const &dict) : iter(dict.dict) {}
    IterC(IterC const &obj) : DMEMB(iter) {}
    IterC& operator= (IterC const &obj) { CMEMB(iter); return *this; }

    bool isDone() const { return iter.isDone(); }
    IterC& next() { iter.next(); return *this; }

    string key() const { return iter.key(); }
    T * value() const { return (T *)iter.value(); }

    int private_getCurrent() const { return iter.private_getCurrent(); }
  };
  friend class IterC;

  class SortedKeyIter {
  private:     // data
    // underlying iterator state
    StringSObjDict<T> const &map;
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
    SortedKeyIter(StringSObjDict<T> const &map0)
      : map(map0)
      , keyIndex(0)
      , numEntries(map.size())
      , sortedKeys(new char const *[numEntries])
    {
      int i = 0;
      // delegate to the other Iter class
      for(IterC iter(map); !iter.isDone(); iter.next()) {
//          xassert(i<numEntries);
        xfailure("is this called?");

        // BUG: This is obviously wrong.  WTH?
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
    T const * value() const {
      return (T const * )map.queryfC(key());
    }
  };

private:    // data
  // underlying dictionary functionality
  StringVoidDict dict;

public:     // funcs
  StringSObjDict() : dict() {}
  ~StringSObjDict() {}

public:    // funcs
  StringSObjDict(StringSObjDict const &obj) : dict(obj.dict) {}

  // comparison and assignment use *pointer* comparison/assignment

  StringSObjDict& operator= (StringSObjDict const &obj)    { dict = obj.dict; return *this; }

  bool operator== (StringSObjDict const &obj) const        { return dict == obj.dict; }
  NOTEQUAL_OPERATOR(StringSObjDict)

  // due to similarity with StringVoidDict, see svdict.h for
  // details on these functions' interfaces

public:
  // ------- selectors ---------
  int size() const                                     { return dict.size(); }

  bool isEmpty() const                                 { return dict.isEmpty(); }
  bool isNotEmpty() const                              { return !isEmpty(); }

  bool query(char const *key, T * &value) const         { return dict.query(key, (void*&)value); }
  T * queryf(char const *key) const                     { return (T *)dict.queryf(key); }
  T * queryif(char const *key) const                    { return (T *)dict.queryif(key); }

  // parallel functions for API consistency
  bool queryC(char const *key, T * &value) const { return query(key, value); }
  T * queryfC(char const *key) const { return queryf(key); }

  bool isMapped(char const *key) const                 { return dict.isMapped(key); }

  // -------- mutators -----------
  void add(char const *key, T * value)                  { dict.add(key, (void*)value); }
  T * /*owner*/ remove(char const *key)                { return (T *)dict.remove(key); }
  T * modify(char const *key, T * newValue)              { return (T *)dict.modify(key, (void*)newValue); }

  void empty()                                         { dict.empty(); }

  // -------- parallel interface for 'rostring' --------
  bool query(rostring key, T * &value) const { return query(key.c_str(), value); }
  T * queryf(rostring key) const             { return queryf(key.c_str()); }
  T * queryif(rostring key) const            { return queryif(key.c_str()); }
  bool isMapped(rostring key) const         { return isMapped(key.c_str()); }
  void add(rostring key, T * value)          { dict.add(key, (void*)value); }
  T * modify(rostring key, T * newValue)      { return modify(key.c_str(), newValue); }
  T * remove(rostring key)                   { return remove(key.c_str()); }

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


#endif // STRSOBJLIST_H
