// sobjlist.h
// serf list of arbitrary objects

// Author: Scott McPeak, 2000

#ifndef __SOBJLIST_H
#define __SOBJLIST_H

#include "voidlist.h"    // VoidList


// forward declarations of template classes, so we can befriend them in SObjList
// (not required by Borland C++ 4.5, but GNU wants it...)
template <class T> class SObjListIter;
template <class T> class SObjListMutator;


// the list is considered to not own any of the items; it's ok to
// insert items multiple times or into multiple lists
template <class T>
class SObjList {
private:
  friend class SObjListIter<T>;
  friend class SObjListMutator<T>;

protected:
  VoidList list;                        // list itself

private:
  SObjList(SObjList const &obj) {}      // not allowed

public:
  SObjList()                            :list() {}
  ~SObjList()                           {}    // all items removed

  // The difference function should return <0 if left should come before
  // right, 0 if they are equivalent, and >0 if right should come before
  // left.  For example, if we are sorting numbers into ascending order,
  // then 'diff' would simply be subtraction.
  typedef int (*Diff)(T const *left, T const *right, void *extra);

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
  void prepend(T *newitem)              { list.prepend(newitem); }
  void append(T *newitem)               { list.append(newitem); }
  void insertAt(T *newitem, int index)  { list.insertAt(newitem, index); }
  void insertSorted(T *newitem, Diff diff, void *extra=NULL)
    { list.insertSorted(newitem, (VoidDiff)diff, extra); }

  // removal
  T *removeAt(int index)                { return (T*)list.removeAt(index); }
  void removeAll()                      { list.removeAll(); }

  // list-as-set: selectors
  int indexOf(T const *item) const      { return list.indexOf((void*)item); }
  bool contains(T const *item) const    { return list.contains((void*)item); }

  // list-as-set: mutators
  bool prependUnique(T *newitem)        { return list.prependUnique(newitem); }
  bool appendUnique(T *newitem)         { return list.appendUnique(newitem); }
  void removeItem(T const *item)        { list.removeItem((void*)item); }    // whether the arg should be const is debatable..
  bool removeIfPresent(T const *item)   { return list.removeIfPresent((void*)item); }

  // complex modifiers
  void reverse()                                    { list.reverse(); }
  void insertionSort(Diff diff, void *extra=NULL)   { list.insertionSort((VoidDiff)diff, extra); }
  void mergeSort(Diff diff, void *extra=NULL)       { list.mergeSort((VoidDiff)diff, extra); }

  // and a related test
  bool isSorted(Diff diff, void *extra=NULL) const  { return list.isSorted((VoidDiff)diff, extra); }

  // multiple lists
  void concat(SObjList &tail)                       { list.concat(tail.list); }
  void appendAll(SObjList const &tail)              { list.appendAll(tail.list); }
  SObjList& operator= (SObjList const &src)         { list = src.list; return *this; }

  // equal items in equal positions
  bool equalAsLists(SObjList const &otherList, Diff diff, void *extra=NULL) const
    { return list.equalAsLists(otherList.list, (VoidDiff)diff, extra); }

  // last-as-set: comparisons (NOT efficient)
  bool equalAsSets(SObjList const &otherList, Diff diff, void *extra=NULL) const
    { return list.equalAsSets(otherList.list, (VoidDiff)diff, extra); }
  bool isSubsetOf(SObjList const &otherList, Diff diff, void *extra=NULL) const
    { return list.isSubsetOf(otherList.list, (VoidDiff)diff, extra); }
  bool containsByDiff(T const *item, Diff diff, void *extra=NULL) const
    { return list.containsByDiff((void*)item, (VoidDiff)diff, extra); }

  // treating the pointer values themselves as the basis for comparison
  bool equalAsPointerLists(SObjList const &otherList) const
    { return list.equalAsPointerLists(otherList.list); }
  bool equalAsPointerSets(SObjList const &otherList) const
    { return list.equalAsPointerSets(otherList.list); }

  // debugging
  bool invariant() const                { return list.invariant(); }
};


// for traversing the list and modifying it (nodes and/or structure)
// NOTE: no list-modification fns should be called on 'list' while this
//       iterator exists, and only one such iterator should exist for
//       any given list
template <class T>
class SObjListMutator {
  friend SObjListIter<T>;

protected:
  VoidListMutator mut;       // underlying mutator

public:
  SObjListMutator(SObjList<T> &lst)     : mut(lst.list) { reset(); }
  ~SObjListMutator()                    {}

  void reset()                          { mut.reset(); }

  // iterator copying; safe *only* until one of the mutators modifies
  // the list structure (by inserting or removing), at which time all
  // other iterators might be in limbo
  SObjListMutator(SObjListMutator const &obj)             : mut(obj.mut) {}
  SObjListMutator& operator=(SObjListMutator const &obj)  { mut = obj.mut;  return *this; }
    // requires that 'this' and 'obj' already refer to the same 'list'

  // iterator actions
  bool isDone() const                   { return mut.isDone(); }
  void adv()                            { mut.adv(); }
  T *data()                             { return (T*)mut.data(); }

  // insertion
  void insertBefore(T *item)            { mut.insertBefore(item); }
    // 'item' becomes the new 'current', and the current 'current' is
    // pushed forward (so the next adv() will make it current again)

  void insertAfter(T *item)             { mut.insertAfter(item); }
    // 'item' becomes what we reach with the next adv();
    // isDone() must be false

  void append(T *item)                  { mut.append(item); }
    // only valid while isDone() is true, it inserts 'item' at the end of
    // the list, and advances such that isDone() remains true; equivalent
    // to { xassert(isDone()); insertBefore(item); adv(); }

  // removal
  T *remove()                           { return (T*)mut.remove(); }
    // 'current' is removed from the list and returned, and whatever was
    // next becomes the new 'current'

  // debugging
  bool invariant() const                { return mut.invariant(); }
};

#define SMUTATE_EACH_OBJLIST(T, list, iter) \
  for(SObjListMutator<T> iter(list); !iter.isDone(); iter.adv())


// for traversing the list without modifying it (neither nodes nor structure)
// NOTE: no list-modification fns should be called on 'list' while this
//       iterator exists
template <class T>
class SObjListIter {
protected:
  VoidListIter iter;      // underlying iterator

public:
  SObjListIter(SObjList<T> const &list) : iter(list.list) {}
  ~SObjListIter()                       {}

  void reset(SObjList<T> const &list)   { iter.reset(list.list); }

  // iterator copying; generally safe
  SObjListIter(SObjListIter const &obj)             : iter(obj.iter) {}
  SObjListIter& operator=(SObjListIter const &obj)  { iter = obj.iter;  return *this; }

  // but copying from a mutator is less safe; see above
  SObjListIter(SObjListMutator<T> &obj)             : iter(obj.mut) {}

  // iterator actions
  bool isDone() const                   { return iter.isDone(); }
  void adv()                            { iter.adv(); }
  T const *data() const                 { return (T const*)iter.data(); }
};

#define SFOREACH_OBJLIST(T, list, iter) \
  for(SObjListIter<T> iter(list); !iter.isDone(); iter.adv())


#endif // __SOBJLIST_H
