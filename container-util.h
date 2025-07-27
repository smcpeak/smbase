// container-util.h
// Utilities for C++ containers.

#ifndef SMBASE_CONTAINER_UTIL_H
#define SMBASE_CONTAINER_UTIL_H

#include "sm-macros.h"                 // OPEN_NAMESPACE, IMEMBFP
#include "xassert.h"                   // xassert


OPEN_NAMESPACE(smbase)


// Return true if 'container' contains 'value'.
template <class CONTAINER, class VALUE>
bool contains(CONTAINER const &container, VALUE const &value)
{
  return container.find(value) != container.end();
}


// Insert 'value' into 'container', insisting that it not already be
// there.
template <class CONTAINER, class VALUE>
void insertUnique(CONTAINER &container, VALUE const &value)
{
  auto it = container.insert(value);
  xassert(it.second);
}


// Expands to the head of a 'for' loop that iterates over all of the
// elements in 'container', with 'iterator' as the iterator variable.
//
// Range-based `for` makes this obsolete; do not use in new code.
#define CONTAINER_FOREACH(container, iterator) \
  for (auto iterator = (container).begin();    \
       iterator != (container).end();          \
       ++iterator)


// Iterate in reverse order for a container that supports that.
//
// Range-based `for` plus `reverseIterRange` makes this obsolete; do not
// use in new code.
#define CONTAINER_REVERSE_FOREACH(container, iterator) \
  for (auto iterator = (container).rbegin();           \
       iterator != (container).rend();                 \
       ++iterator)


// Container wrapper to yield `rbegin/rend` for reverse iteration when
// the container supports it.
template <typename CONTAINER>
class ReverseIterRange {
public:      // data
  CONTAINER &m_container;

public:      // methods
  ReverseIterRange(CONTAINER &container)
    : IMEMBFP(container) {}

  auto begin() const { return m_container.rbegin(); }
  auto end() const { return m_container.rend(); }
};

// Function to call within range-based `for` for reverse iteration.
template <typename CONTAINER>
ReverseIterRange<CONTAINER> reverseIterRange(CONTAINER &container)
{
  return ReverseIterRange(container);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_CONTAINER_UTIL_H
