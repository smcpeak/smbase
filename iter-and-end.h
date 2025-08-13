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


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ITER_AND_END_H
