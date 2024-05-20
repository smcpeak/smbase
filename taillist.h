// taillist.h; see license.txt for copyright and terms of use
// List wrapper around VoidTailList, like ASTList, but it doesn't own
// the elements.

// taken almost verbatim from asttlist.h in smbase

#ifndef TAILLIST_H
#define TAILLIST_H

#include "vdtllist.h"           // VoidTailList

template <class T> class TailListIter;
template <class T> class TailListIterNC;

// a list which does not own the items in it (will NOT deallocate
// them), and has constant-time access to the last element
template <class T>
class TailList {
private:
  friend class TailListIter<T>;
  friend class TailListIterNC<T>;

protected:
  VoidTailList list;            // list itself

private:
  TailList(TailList const &obj); // not allowed

public:
  TailList()                             : list() {}
  ~TailList()                            {  }

  // ctor to make singleton list; often quite useful
  TailList(T *elt)                       : list() { prepend((void*)elt); }

  // If 'src' is not nullptr, this constructor first "steals" all of its
  // elements, then deallocates the 'src' object itself.  Otherwise, it
  // simply constructs an empty list.
  explicit TailList(TailList<T> * /*nullable owner*/ src)
    : list()
  {
    if (src) {
      list.stealElements(&src->list);
      delete src;
    }
  }

  // First, remove all elements from 'this'.  Then, if 'src' is not
  // nullptr, transfer all of its elements to 'this', leaving it empty.
  void stealElements(TailList<T> * /*nullable*/ src)
  {
    removeAll();
    if (src) {
      list.stealElements(src);
    }
  }

  // Empty 'this', then steal all of the elements from 'src', and
  // finally deallocate the 'src' object itself.
  void steal(TailList<T> * /*nullable owner*/ src)
  {
    stealElements(src);
    if (src) {
      delete src;
    }
  }

  // selectors
  int count() const                     { return list.count(); }
  bool isEmpty() const                  { return list.isEmpty(); }
  bool isNotEmpty() const               { return list.isNotEmpty(); }
  T *nth(int which)                     { return (T*)list.nth(which); }
  T const *nthC(int which) const        { return (T const*)list.nth(which); }
  T *first()                            { return (T*)list.first(); }
  T const *firstC() const               { return (T const*)list.first(); }
  T *last()                             { return (T*)list.last(); }
  T const *lastC() const                { return (T const*)list.last(); }

  // insertion
  void prepend(T *newitem)              { list.prepend((void*)newitem); }
  void append(T *newitem)               { list.append((void*)newitem); }
  void appendAll(TailList<T> &tail)     { list.appendAll(tail.list); }
  void insertAt(T *newitem, int index)  { list.insertAt((void*)newitem, index); }
  void concat(TailList<T> &tail)         { list.concat(tail.list); }

  // removal
  T *removeFirst()                      { return (T*)list.removeFirst(); }
  T *removeLast()                       { return (T*)list.removeLast(); }
  T *removeAt(int index)                { return (T*)list.removeAt(index); }
  void removeItem(T *item)              { list.removeItem((void*)item); }
  void removeAll()                      { list.removeAll(); }

  // list-as-set: selectors
  int indexOf(T const *item) const      { return list.indexOf((void*)item); }
  int indexOfF(T const *item) const     { return list.indexOfF((void*)item); }
  bool contains(T const *item) const    { return list.contains((void*)item); }

  // list-as-set: mutators
  bool prependUnique(T *newitem)        { return list.prependUnique((void*)newitem); }
  bool appendUnique(T *newitem)         { return list.appendUnique((void*)newitem); }

  // debugging: two additional invariants
  void selfCheck() const                { list.selfCheck(); }
};


template <class T>
class TailListIter {
protected:
  VoidTailListIter iter;        // underlying iterator

public:
  TailListIter() {}      // initially done
  TailListIter(TailList<T> const &list) : iter(list.list) {}
  ~TailListIter()                       {}

  void reset(TailList<T> const &list)   { iter.reset(list.list); }

  // iterator copying; generally safe
  TailListIter(TailListIter const &obj)             : iter(obj.iter) {}
  TailListIter& operator=(TailListIter const &obj)  { iter = obj.iter;  return *this; }

  // iterator actions
  bool isDone() const                   { return iter.isDone(); }
  void adv()                            { iter.adv(); }
  T const *data() const                 { return (T const*)iter.data(); }
};

#define FOREACH_TAILLIST(T, list, iter) \
  for(TailListIter<T> iter(list); !iter.isDone(); iter.adv())


// version of the above, but for non-const-element traversal
template <class T>
class TailListIterNC {
protected:
  VoidTailListIter iter;        // underlying iterator

public:
  TailListIterNC() {}      // initially done
  TailListIterNC(TailList<T> &list)      : iter(list.list) {}
  ~TailListIterNC()                     {}

  void reset(TailList<T> &list)         { iter.reset(list.list); }

  // iterator copying; generally safe
  TailListIterNC(TailListIterNC const &obj)             : iter(obj.iter) {}
  TailListIterNC& operator=(TailListIterNC const &obj)  { iter = obj.iter;  return *this; }

  // iterator actions
  bool isDone() const                   { return iter.isDone(); }
  void adv()                            { iter.adv(); }
  T *data() const                       { return (T*)iter.data(); }

  // iterator mutation; use with caution
  void setDataLink(T *newData)          { iter.setDataLink((void*)newData); }
};

#define FOREACH_TAILLIST_NC(T, list, iter) \
  for(TailListIterNC<T> iter(list); !iter.isDone(); iter.adv())

#endif // TailLIST_H
