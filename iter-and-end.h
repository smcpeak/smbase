// iter-and-end.h
// `IterAndEnd` and `ConstIterAndEnd` for representing iterator and end.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_ITER_AND_END_H
#define SMBASE_ITER_AND_END_H

#include "iter-and-end-fwd.h"          // fwds for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP, etc.


OPEN_NAMESPACE(smbase)


// TODO: Move declarations into an iface file.


// Iterator, with end, that can modify its container.
template <typename CONTAINER>
class IterAndEnd {
public:      // types
  // The type of iterator we wrap.
  using Iterator = typename CONTAINER::iterator;

  // Value of the elements we iterate over.
  using value_type = typename Iterator::value_type;

public:      // data
  // The iterator.
  Iterator m_iter;

  // The end.  Ordinarily, this does not change after initialization,
  // but I do not mark it `const` because that would preclude
  // assignment.
  Iterator m_end;

public:      // methods
  IterAndEnd(Iterator iter, Iterator end)
    : IMEMBFP(iter),
      IMEMBFP(end)
  {}

  IterAndEnd(IterAndEnd const &obj)
    : DMEMB(m_iter),
      DMEMB(m_end)
  {}

  IterAndEnd &operator=(IterAndEnd const &obj)
  {
    CMEMB(m_iter);
    CMEMB(m_end);
    return *this;
  }

  bool operator==(IterAndEnd const &obj) const
  {
    return EMEMB(m_iter) &&
           EMEMB(m_end);
  }

  bool operator!=(IterAndEnd const &obj) const
  {
    return !operator==(obj);
  }

  // -------------------- container-like interface ---------------------
  bool empty() const
  {
    return m_iter == m_end;
  }

  Iterator begin() const
  {
    return m_iter;
  }

  Iterator end() const
  {
    return m_end;
  }

  // --------------------- iterator-like interface ---------------------
  value_type &operator*() const
  {
    return *m_iter;
  }

  // No `operator->` because applying `->` to iterators is generally
  // troublesome.

  IterAndEnd &operator++()
  {
    ++m_iter;
    return *this;
  }

  IterAndEnd operator++(int)
  {
    IterAndEnd ret(*this);
    ++m_iter;
    return ret;
  }
};


// Extract begin/end from `c`.
template <typename CONTAINER>
IterAndEnd<CONTAINER> iterAndEnd(CONTAINER &c)
{
  return IterAndEnd<CONTAINER>(c.begin(), c.end());
}


// Iterator, with end, that *cannot* modify its container.
template <typename CONTAINER>
class ConstIterAndEnd {
public:      // types
  // The type of iterator we wrap.
  using Iterator = typename CONTAINER::const_iterator;

  // Value of the elements we iterate over.
  using value_type = typename Iterator::value_type;

public:      // data
  // The iterator.
  Iterator m_iter;

  // The end.  Ordinarily, this does not change after initialization,
  // but I do not mark it `const` because that would preclude
  // assignment.
  Iterator m_end;

public:      // methods
  ConstIterAndEnd(Iterator iter, Iterator end)
    : IMEMBFP(iter),
      IMEMBFP(end)
  {}

  ConstIterAndEnd(ConstIterAndEnd const &obj)
    : DMEMB(m_iter),
      DMEMB(m_end)
  {}

  ConstIterAndEnd &operator=(ConstIterAndEnd const &obj)
  {
    CMEMB(m_iter);
    CMEMB(m_end);
    return *this;
  }

  // Assuming that `CONTAINER::const_iterator` can be created from a
  // `CONTAINER::iterator`, we should be able to do the same for our
  // combined structure.
  ConstIterAndEnd(IterAndEnd<CONTAINER> const &obj)
    : DMEMB(m_iter),
      DMEMB(m_end)
  {}

  bool operator==(ConstIterAndEnd const &obj) const
  {
    return EMEMB(m_iter) &&
           EMEMB(m_end);
  }

  bool operator!=(ConstIterAndEnd const &obj) const
  {
    return !operator==(obj);
  }

  // -------------------- container-like interface ---------------------
  bool empty() const
  {
    return m_iter == m_end;
  }

  Iterator begin() const
  {
    return m_iter;
  }

  Iterator end() const
  {
    return m_end;
  }

  // --------------------- iterator-like interface ---------------------
  value_type const &operator*() const
  {
    return *m_iter;
  }

  // No `operator->` because applying `->` to iterators is generally
  // troublesome.

  ConstIterAndEnd &operator++()
  {
    ++m_iter;
    return *this;
  }

  ConstIterAndEnd operator++(int)
  {
    ConstIterAndEnd ret(*this);
    ++m_iter;
    return ret;
  }
};


// Extract begin/end from `c`.
template <typename CONTAINER>
ConstIterAndEnd<CONTAINER> constIterAndEnd(CONTAINER const &c)
{
  return ConstIterAndEnd<CONTAINER>(c.begin(), c.end());
}


// Compare the values pointed to by `a` and `b` using `isLessThan`,
// except that if either is empty, consider the non-empty one to be
// less; and if they are both empty, then they are equal.
template <typename COMPARATOR, typename ITER_AND_END>
int compareIterAndEnds(
  COMPARATOR const &isLessThan,
  ITER_AND_END const &a,
  ITER_AND_END const &b)
{
  // If one is empty and the other is not, the non-empty one is
  // considered less.
  bool aEmpty = a.empty();
  bool bEmpty = b.empty();
  if (aEmpty < bEmpty) {
    // `aEmpty` is false and `bEmpty` is true, so `b` is empty, thus `a`
    // is treater as less.
    return -1;
  }
  else if (aEmpty > bEmpty) {
    return +1;
  }

  if (aEmpty) {
    // Both are empty.
    return 0;
  }
  else {
    // Both are non-empty, so compare elements.
    if (isLessThan(*a, *b)) {
      return -1;
    }
    else if (isLessThan(*b, *a)) {
      return +1;
    }
    else {
      return 0;
    }
  }
}


// Compare the contents of `a` and `b` lexicographically.  If they are
// not equal, `a` and `b` will left at the first point of difference.
template <typename COMPARATOR, typename ITER_AND_END>
int compareLexicographicallyIAE(
  COMPARATOR const &isLessThan,
  ITER_AND_END &a /*INOUT*/,
  ITER_AND_END &b /*INOUT*/)
{
  // Traverse the iterators in parallel, comparing elements.
  while (!( a.empty() && b.empty() )) {
    int res = compareIterAndEnds(isLessThan, a, b);
    if (res != 0) {
      // Either exactly one container is exhausted, or the corresponding
      // elements are unequal.
      return res;
    }

    ++a;
    ++b;
  }

  return 0;
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ITER_AND_END_H
