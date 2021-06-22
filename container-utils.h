// container-utils.h
// Utilities for C++ containers.

#ifndef SMBASE_CONTAINER_UTILS_H
#define SMBASE_CONTAINER_UTILS_H


// Return true if 'container' contains 'value'.
template <class CONTAINER, class VALUE>
bool contains(CONTAINER const &container, VALUE const &value)
{
  return container.find(value) != container.end();
}


// Expands to the head of a 'for' loop that iterates over all of the
// elements in 'container', with 'iterator' as the iterator variable.
#define CONTAINER_FOREACH(container, iterator) \
  for (auto iterator = (container).begin();    \
       iterator != (container).end();          \
       ++iterator)


#endif // SMBASE_CONTAINER_UTILS_H
