// container-util.h
// Utilities for C++ containers.

#ifndef SMBASE_CONTAINER_UTIL_H
#define SMBASE_CONTAINER_UTIL_H

#include "xassert.h"                   // xassert


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
#define CONTAINER_FOREACH(container, iterator) \
  for (auto iterator = (container).begin();    \
       iterator != (container).end();          \
       ++iterator)


// Iterate in reverse order for a container that supports that.
#define CONTAINER_REVERSE_FOREACH(container, iterator) \
  for (auto iterator = (container).rbegin();           \
       iterator != (container).rend();                 \
       ++iterator)


#endif // SMBASE_CONTAINER_UTIL_H
