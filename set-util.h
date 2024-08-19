// set-util.h
// Utilities related to `std::set`.

#ifndef SMBASE_SET_UTIL_H
#define SMBASE_SET_UTIL_H

#include "set-util-iface.h"            // interface for this module

#include "xassert.h"                   // xassert

#include <optional>                    // std::optional
#include <ostream>                     // std::ostream
#include <set>                         // std::set
#include <vector>                      // std::vector


// NOTE: Comments describing these functions are in `set-util-iface.h`.


template <class T>
bool setInsert(std::set<T> &s, T const &t)
{
  auto res = s.insert(t);
  return res.second;
}


template <class T>
void setInsertUnique(std::set<T> &s, T const &t)
{
  bool inserted = setInsert(s, t);
  xassert(inserted);
}


template <class T>
bool setInsertAll(std::set<T> &dest, std::set<T> const &src)
{
  bool ret = false;

  for (auto const &v : src) {
    ret |= setInsert(dest, v);
  }

  return ret;
}


template <class T>
bool setContains(stdfwd::set<T> const &s, T const &t)
{
  return s.find(t) != s.end();
}


template <class T>
bool isSubsetOf(std::set<T> const &subset, std::set<T> const &superset)
{
  for (auto it = subset.begin(); it != subset.end(); ++it) {
    auto it2 = superset.find(*it);
    if (it2 == superset.end()) {
      // '*it' is in 'subset' but not in 'superset'.
      return false;
    }
  }
  return true;
}


template <class T>
bool isSubsetOf_getExtra(T &extra /*OUT*/,
                         std::set<T> const &smaller,
                         std::set<T> const &larger)
{
  for (T const &element : smaller) {
    if (!setContains(larger, element)) {
      extra = element;
      return false;
    }
  }

  return true;
}


template <class T>
std::optional<T> setHasElementNotIn(
  std::set<T> const &smaller,
  std::set<T> const &larger)
{
  for (T const &element : smaller) {
    if (!setContains(larger, element)) {
      return std::make_optional(element);
    }
  }

  return std::nullopt;
}


template <typename OELT, typename IELT, typename FUNC>
std::set<OELT> setMapElements(std::set<IELT> const &input,
                              FUNC const &func)
{
  std::set<OELT> output;

  for (IELT const &ielt : input) {
    output.insert(func(ielt));
  }

  return output;
}


template <class T>
std::vector<T> setToVector(std::set<T> const &s)
{
  std::vector<T> ret;
  for (T const &elt : s) {
    ret.push_back(elt);
  }
  return ret;
}


template <class T, class PRINT_ELEMENT>
void setWrite(
  std::ostream &os,
  std::set<T> const &s,
  PRINT_ELEMENT const &printElement)
{
  os << '{';

  int ct = 0;
  for (auto const &e : s) {
    if (ct > 0) {
      os << ", ";
    }
    printElement(os, e);
    ++ct;
  }

  os << '}';
}


template <class T>
std::ostream& operator<< (std::ostream &os, std::set<T> const &s)
{
  setWrite(os, s,
    [](std::ostream &os, T const &t) -> void {
      os << t;
    });
  return os;
}


template <class T, class PRINT_ELEMENT>
SetWriter<T,PRINT_ELEMENT>::SetWriter(
  std::set<T> const &s,
  PRINT_ELEMENT const &pe)
  : m_set(s),
    m_printElement(pe)
{}


template <class T, class PRINT_ELEMENT>
void SetWriter<T,PRINT_ELEMENT>::write(std::ostream &os) const
{
  setWrite(os, m_set, m_printElement);
}


template <class T, class PRINT_ELEMENT>
SetWriter<T,PRINT_ELEMENT> setWriter(
  std::set<T> const &s,
  PRINT_ELEMENT const &pe)
{
  return SetWriter<T,PRINT_ELEMENT>(s, pe);
}


#endif // SMBASE_SET_UTIL_H
