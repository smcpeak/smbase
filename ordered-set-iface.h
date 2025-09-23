// ordered-set-iface.h
// Interface for `ordered-set` module.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_ORDERED_SET_IFACE_H
#define SMBASE_ORDERED_SET_IFACE_H

#include "ordered-set-fwd.h"           // fwds for this module

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [n]
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <initializer_list>            // std::initializer_list [n]
#include <map>                         // std::map
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


/* A sequence of `T`, indexed by (presumably integral) type `INDEX`,
   where every element is unique.  This supports efficient membership
   test and insertion (at the end), but not removal or insertion in the
   middle.

   The primary intended application is to provide unique numeric
   identifiers for a set of larger objects (such as strings).
*/
template <typename T, typename INDEX>
class OrderedSet {
public:      // types
  // Iterate in sequential order.
  class const_iterator {
    friend OrderedSet;

  private:     // data
    // Container being iterated over.
    OrderedSet const &m_set;

    // Index of the current element.  When this equals `m_set.size()`,
    // this is the end iterator position.
    INDEX m_index;

  private:     // methods
    explicit const_iterator(OrderedSet const &set, INDEX index);

  public:      // methods
    const_iterator(const_iterator const &obj) = default;

    // Requires: `*this` and `obj` refer to the same container.
    const_iterator &operator=(const_iterator const &obj);

    bool operator==(const_iterator const &obj) const;
    bool operator!=(const_iterator const &obj) const
      { return !operator==(obj); }

    T const &operator*() const;

    // No operator-> on purpose.
    //
    // TODO: Write up an explanation.

    const_iterator &operator++();
    const_iterator operator++(int);
  };

private:     // data
  // Primary storage for `T` elements, which also serves as the map from
  // element to index.
  std::map<T, INDEX> m_eltToIndex;

  // Map from index to element, as a pointer into `m_eltToIndex`.
  //
  // Invariant: The elements of `m_vec` correspond 1-1 to the elements
  // of `m_eltToIndex`, being pointers to the latter's keys.
  std::vector<T const *> m_indexToElt;

private:     // methods
  // Rebuild `m_indexToElt` from scratch by reading `m_eltToIndex`.
  void rebuildIndexToElt();

public:      // methods
  ~OrderedSet();

  // Empty container.
  OrderedSet();

  OrderedSet(OrderedSet const &obj);
  OrderedSet(OrderedSet &&obj);

  OrderedSet(std::initializer_list<T> ilist);

  // Assert invariants.
  void selfCheck() const;

  // ----------------------------- Queries -----------------------------
  // Compare as a sequence in lexicographical order by `T`.
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(OrderedSet)

  // True if the container has no elements.
  bool empty() const;

  // Number of contained elements.
  INDEX size() const;

  // True if `t` is in the set.
  bool contains(T const &t) const;

  // Return the element at `index`.  The reference is invalidated by any
  // non-const method.
  //
  // Requires: 0 <= index < size()
  T const &atC(INDEX index) const;

  // Get the index of element `t`.
  //
  // Requires: contains(t)
  INDEX getIndex(T const &t) const;

  // Iterate in sequence order (index 0, index 1, etc.).
  const_iterator cbegin() const;
  const_iterator cend() const;
  const_iterator begin() const    { return cbegin(); }
  const_iterator end() const      { return cend(); }

  // Return the elements as a sequence (untagged).
  operator gdv::GDValue() const;

  // -------------------------- Modifications --------------------------
  OrderedSet &operator=(OrderedSet const &obj);
  OrderedSet &operator=(OrderedSet &&obj);

  // Reset to an empty container.
  void clear();

  // If `t` is not already in the set, add it to the end and return its
  // associated index.  If it is, return the already-associated index.
  INDEX insert(T const &t);

  // Same, but asserting that `t` is not already in the set.
  INDEX insertUnique(T const &t);

  void swapWith(OrderedSet &obj);
  friend void swap(OrderedSet &a, OrderedSet &b)
    { a.swapWith(b); }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_ORDERED_SET_IFACE_H
