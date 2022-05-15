// strictly-sorted.h
// is_strictly_sorted function.

#ifndef SMBASE_STRICTLY_SORTED_H
#define SMBASE_STRICTLY_SORTED_H

// This is like std::is_sorted, except it requires a strict ordering.
//
// This could be implemented in terms of std::is_sorted by flipping the
// order of the arguments to Compare and negating its result, but this
// is easy to do directly as well.
template <class ForwardIterator, class Compare>
bool is_strictly_sorted(
  ForwardIterator begin,
  ForwardIterator end,
  Compare compare)
{
  if (begin == end) {
    return true;
  }

  // Have 'prev' follow 'cur' by one element.
  ForwardIterator prev(begin);
  ForwardIterator cur(prev);
  ++cur;

  while (cur != end) {
    // When strictly sorted, '*prev < *cur'.
    if (!compare(*prev, *cur)) {
      return false;
    }

    ++prev;
    ++cur;
  }

  return true;
}

#endif // SMBASE_STRICTLY_SORTED_H
