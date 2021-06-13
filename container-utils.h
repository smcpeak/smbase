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


#endif // SMBASE_CONTAINER_UTILS_H
