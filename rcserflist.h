// rcserflist.h
// Array-backed sequence of reference-counted pointers.

#ifndef RCSERFLIST_H
#define RCSERFLIST_H

#include "array.h"                     // ArrayStack
#include "refct-serf.h"                // RCSerf
#include "sm-macros.h"                 // NO_OBJECT_COPIES


// Forward in this file.
template <class T> class RCSerfListIter;
template <class T> class RCSerfListIterNC;


// This is a container with an interface similar to SObjList, except
// storing RCSerf objects.  The interface is not identical, though, as
// I'm actually using an array instead of a linked list to implement
// this.
//
// Also, for the moment the interface is just the minimum I need to
// replace the uses of SObjList<Observer> in the editor.
template <class T>
class RCSerfList {
  friend class RCSerfListIter<T>;
  friend class RCSerfListIterNC<T>;

protected:   // data
  // List elements.
  ArrayStack<RCSerf<T> > m_arr;

public:      // funcs
  RCSerfList()                         : m_arr() {}
  ~RCSerfList()                        {}

  int count() const                    { return m_arr.length(); }
  bool isEmpty() const                 { return m_arr.isEmpty(); }
  bool isNotEmpty() const              { return m_arr.isNotEmpty(); }

  RCSerf<T> &nthRef(int n)             { return m_arr[n]; }

  int indexOf(T const *item) const;
  bool contains(T const *item) const   { return this->indexOf(item) >= 0; }

  // Append an item not already in the list.
  void appendNewItem(T *newItem);

  void removeItem(T const *item);

  void removeAll();
};


template <class T>
int RCSerfList<T>::indexOf(T const *item) const
{
  for (int i=0; i < m_arr.length(); i++) {
    if (m_arr[i] == item) {
      return i;
    }
  }
  return -1;
}


template <class T>
void RCSerfList<T>::appendNewItem(T *newItem)
{
  xassert(!this->contains(newItem));
  m_arr.push(newItem);
}


template <class T>
void RCSerfList<T>::removeItem(T const *item)
{
  int i = this->indexOf(item);
  xassert(i >= 0);
  m_arr.moveElement(i, m_arr.length()-1);

  // This must be done explicitly because 'pop' just moves the end
  // pointer back, leaving the array element untouched.
  m_arr.top() = NULL;

  m_arr.pop();
}


template <class T>
void RCSerfList<T>::removeAll()
{
  while (m_arr.isNotEmpty()) {
    m_arr.top() = NULL;
    m_arr.pop();
  }
}


// Const iterator for RCSerfList.
template <class T>
class RCSerfListIter {
  // Merely not implemented.
  NO_OBJECT_COPIES(RCSerfListIter);

protected:   // data
  // List we are iterating over.
  RCSerfList<T> const &m_list;

  // Index of 'data()' item.
  int m_index;

public:      // funcs
  RCSerfListIter(RCSerfList<T> const &list)
    : m_list(list),
      m_index(0)
  {}

  ~RCSerfListIter()
  {}

  bool isDone() const
  {
    return m_index >= m_list.count();
  }

  void adv()
  {
    xassert(!isDone());
    m_index++;
  }

  T const *data() const
  {
    return m_list.m_arr[m_index];
  }
};

#define FOREACH_RCSERFLIST(T, list, iter) \
  for(RCSerfListIter< T > iter(list); !iter.isDone(); iter.adv())


// Non-const iterator for RCSerfList.  Copied from RCSerfListIter,
// comments deleted to avoid duplication.
template <class T>
class RCSerfListIterNC {
  NO_OBJECT_COPIES(RCSerfListIterNC);

protected:
  RCSerfList<T> &m_list;
  int m_index;

public:
  RCSerfListIterNC(RCSerfList<T> &list)
    : m_list(list),
      m_index(0)
  {}

  ~RCSerfListIterNC()
  {}

  bool isDone() const
  {
    return m_index >= m_list.count();
  }

  void adv()
  {
    xassert(!isDone());
    m_index++;
  }

  T *data() const
  {
    return m_list.m_arr[m_index];
  }
};

#define FOREACH_RCSERFLIST_NC(T, list, iter) \
  for(RCSerfListIterNC< T > iter(list); !iter.isDone(); iter.adv())


#endif // RCSERFLIST_H
