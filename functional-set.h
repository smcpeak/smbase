// functional-set.h
// FunctionalSet, FSElement, and FunctionalSetManager.

#ifndef SMBASE_FUNCTIONAL_SET_H
#define SMBASE_FUNCTIONAL_SET_H

#include "sm-compare.h"                // StrongOrdering
#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "sm-rc-ptr.h"                 // RefCountObject, RCPtr

#include <cstddef>                     // std::size_t
#include <iosfwd>                      // std::ostream
#include <set>                         // std::set
#include <string>                      // std::string
#include <vector>                      // std::vector


// This is an abstract superclass for clients to derive from.  Instances
// of FSElement must be immutable.
class FSElement : public RefCountObject {
public:      // methods
  // FSElements are first partitioned into "kinds", where each kind is
  // represented by a C string, with strings compared with strcmp.
  // Typically, this will be the name of a class.
  virtual char const *fseKind() const = 0;

  // Compare the 'fseKind()' values.
  StrongOrdering compareKinds(FSElement const &obj) const;

  // Compare 'this' to 'obj'.  This method should begin with
  // FSELEMENT_COMPARE_TO_PRELUDE.  Then it can perform type-specific
  // comparison.
  virtual StrongOrdering compareTo(FSElement const &obj) const = 0;

  #define MAKE_FSE_RELOP(op)                              \
    bool operator op (FSElement const &obj) const         \
      { return compareTo(obj) op StrongOrdering::equal; }
  MAKE_FSE_RELOP( == )
  MAKE_FSE_RELOP( != )
  MAKE_FSE_RELOP( <  )
  MAKE_FSE_RELOP( >  )
  MAKE_FSE_RELOP( <= )
  MAKE_FSE_RELOP( >= )
  #undef MAKE_FSE_RELOP

  // Render the value as a string.
  virtual void print(std::ostream &os) const = 0;

  // Capture the result of 'print(ostream)'.
  std::string toString() const;
};

inline std::ostream& operator<< (std::ostream &os, FSElement const &obj)
{
  obj.print(os);
  return os;
}


// This is how a class TYPE derived from FSElement should begin its
// 'compareTo' method.  It assumes the parameter is called 'obj_', and
// declares an appropriately-typed local called 'obj'.
#define FSELEMENT_COMPARETO_PRELUDE(TYPE)         \
  {                                               \
    StrongOrdering ret = compareKinds(obj_);      \
    if (ret != StrongOrdering::equal) {           \
      return ret;                                 \
    }                                             \
  }                                               \
  TYPE const &obj =                               \
    dynamic_cast<TYPE const &>(obj_) /* user ; */


// Represent a set of FSElement elements.  Clients are intended to hold
// a pointer to this class and use reference counting to manage object
// lifetimes.
//
// A set is itself an FSElement, so it is possible to create sets of
// sets, and to compare sets to each other.  Comparison of two sets is
// lexicographic on the sorted element sequences (what 'getElements'
// returns).
//
class FunctionalSet : public FSElement {
  // Set objects should not be copied since they are immutable, so
  // clients can simply copy pointers (with reference counting as
  // needed).
  NO_OBJECT_COPIES(FunctionalSet);

public:      // types
  // Type for element count and indices.
  typedef std::size_t size_type;

private:     // data
  // All elements in the set less than 'm_middle', or NULL if there are
  // no elements to the left.
  RCPtr<FunctionalSet> m_left;

  // Middle element of the set.  If there are N elements in the set, and
  // N >= 1, then 'm_left' contains floor(N/2) elements, 'm_right'
  // contains floor((N-1)/2) elements, and 'm_middle' is the element at
  // index floor(N/2).
  //
  //
  //                            left          middle right
  // Example with N=5 elements: floor(5/2)==2   1    floor(4/2)==2
  // Example with N=4 elements: floor(4/2)==2   1    floor(3/2)==1
  //
  // This is NULL iff the set is empty.
  RCPtr<FSElement> m_middle;

  // All elements in the set greater than 'm_middle', or NULL if there
  // are no elements to the right.
  RCPtr<FunctionalSet> m_right;

  // Number of elements in this set.
  size_type m_size;

private:     // methods
  // Check that all elements are strictly between 'lowBound' and
  // 'highBound', although either can be NULL, which imposes no limit.
  void checkBounds(FSElement const * /*nullable*/ lowBound,
                   FSElement const * /*nullable*/ highBound) const;

public:      // methods
  // Normally, FunctionalSetManager takes care of creating these in
  // order to ensure the same sets are represented by the same objects,
  // but there isn't a particular problem with creating them outside of
  // that context.  The caller must respect the invariants though.
  FunctionalSet(RCPtr<FunctionalSet> left,
                RCPtr<FSElement> middle,
                RCPtr<FunctionalSet> right);

  ~FunctionalSet();

  // Number of elements in this set.
  size_type size() const { return m_size; }

  // True if 'size() == 0'.
  bool empty() const { return m_size == 0; }

  // Get the element at 'index'.
  //
  // Requires: 0 <= index < size().
  FSElement const *at(size_type index) const;

  // True if 'elt' is in this set.
  bool contains(FSElement const *elt) const;

  // Append all elements of this set to 'vec', in order.
  void getElements(std::vector<RCPtr<FSElement> > &vec) const;

  // Check object invariants.  Throw if there is a problem.
  void checkInvariants() const;

  // Check just the size-related invariants.
  void checkSizes() const;

  // FSElement methods.
  char const *fseKind() const override;
  StrongOrdering compareTo(FSElement const &obj) const override;
  void print(std::ostream &os) const override;

  // Print the elements, separated by commas, without enclosing braces.
  void printElements(std::ostream &os) const;

  // Return the number of elements expected on each side, given 'n',
  // the total number of elements.
  static size_type leftSizeForTotal(size_type n)
    { return n/2; }
  static size_type rightSizeForTotal(size_type n)
    { return n? (n-1)/2 : 0; }
};


// Comparison class for RCPtr<FunctionalSet>.
class RCPtrFunctionalSetCompare {
public:      // methods
  bool operator() (RCPtr<FunctionalSet> const &a,
                   RCPtr<FunctionalSet> const &b) const
    { return (*a) < (*b); }
};


// Manage a collection of FunctionalSet objects, where each is a
// unique representative of a particular set, and provide the interface
// for creating such sets.
//
// When this class is destroyed, it will decrement the reference counts
// for all managed sets, but if a client still has outstanding
// references then those sets will remain.
//
class FunctionalSetManager {
  NO_OBJECT_COPIES(FunctionalSetManager);

public:      // types
  // Type for element count and indices.
  typedef FunctionalSet::size_type size_type;

private:     // data
  // All known sets.
  std::set<RCPtr<FunctionalSet>, RCPtrFunctionalSetCompare> m_sets;

  // The empty set.
  //RCPtr<FunctionalSet> m_emptySet;

private:     // methods
  // Like 'setFromVectorRange', but return NULL for the empty set.
  RCPtr<FunctionalSet> nullableSetFromVectorRange(
    std::vector<RCPtr<FSElement> > const &vec,
    std::size_t start, std::size_t end);

public:      // methods
  FunctionalSetManager();
  ~FunctionalSetManager();

  // Build a set out of the elements in 'vec', which must already be
  // strictly sorted.
  RCPtr<FunctionalSet> setFromVector(
    std::vector<RCPtr<FSElement> > const &vec);

  // Same, but using a specific range: '[start,end)'.
  RCPtr<FunctionalSet> setFromVectorRange(
    std::vector<RCPtr<FSElement> > const &vec,
    size_type start, size_type end);

  // Get the empty set.  This is *not* a NULL pointer, so the
  // FunctionalSet methods are all available.
  RCPtr<FunctionalSet> emptySet();

  // Get a set with one element
  RCPtr<FunctionalSet> singleton(RCPtr<FSElement> elt);

  // Union of two sets.
  RCPtr<FunctionalSet> unionSet(RCPtr<FunctionalSet> a,
                                RCPtr<FunctionalSet> b);

  // Intersection of two sets.
  RCPtr<FunctionalSet> intersection(RCPtr<FunctionalSet> a,
                                    RCPtr<FunctionalSet> b);

  // Check object invariants.  Throw if there is a problem.  This checks
  // invariants for all known sets.
  void checkInvariants() const;
};


#endif // SMBASE_FUNCTIONAL_SET_H
