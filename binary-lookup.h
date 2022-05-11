// binary-lookup.h
// binary_lookup function.

#ifndef BINARY_LOOKUP_H
#define BINARY_LOOKUP_H

#include <algorithm>                   // std::lower_bound


// Return the first element in '[begin,end)' that compares equal to
// 'value'.
//
// This is similar to 'std::binary_search', except we get the specific
// element, which is useful when the value is effectively a key of a
// map, rather than an element of a set.
template <class ForwardIterator, class T, class Compare>
ForwardIterator binary_lookup(
  ForwardIterator begin,
  ForwardIterator end,
  T const &value,
  Compare lessThan)
{
  // Get the first element that is not less than 'value'.
  ForwardIterator first = std::lower_bound(begin, end, value, lessThan);

  // If anything was found, we know '!(first<value)', i.e.,
  // 'value<=first'.  So if also '!(value<first)', meaning
  // 'first<=value', then 'value==first'.
  if (first != end && !lessThan(value, *first)) {
    return first;
  }
  else {
    // Not found.
    return end;
  }
}


#endif // BINARY_LOOKUP_H
