// strcmp-compare.h
// Comparison functions based on strcmp.

#ifndef STRCMP_COMPARE_H
#define STRCMP_COMPARE_H

#include <string.h>                    // strcmp


// True if 'a<b' in strcmp order.
inline bool strcmpCompare(char const *a, char const *b)
{
  return strcmp(a,b) < 0;
}


// True if 'b<a' in strcmp order.
inline bool strcmpRevCompare(char const *a, char const *b)
{
  return strcmp(b,a) < 0;
}


// Comparison object for STL algorithms and containers.
class StrcmpCompare {
public:
  bool operator() (char const *a, char const *b) {
    return strcmpCompare(a, b);
  }
};


class StrcmpRevCompare {
public:
  bool operator() (char const *a, char const *b) {
    return strcmpRevCompare(a, b);
  }
};


#endif // STRCMP_COMPARE_H
