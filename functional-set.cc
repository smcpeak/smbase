// functional-set.cc
// Code for functional-set.h.

#include "functional-set.h"            // this module

#include "xassert.h"                   // xassert

#include <iostream>                    // std::ostream
#include <sstream>                     // std::ostringstream


// --------------------------- FSElement -------------------------------
StrongOrdering FSElement::compareKinds(FSElement const &obj) const
{
  return strongOrder(this->fseKind(), obj.fseKind());
}


std::string FSElement::toString() const
{
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}


// ------------------------- FunctionalSet -----------------------------
FunctionalSet::FunctionalSet(RCPtr<FunctionalSet> left,
                             RCPtr<FSElement> middle,
                             RCPtr<FunctionalSet> right)
  : m_left(left),
    m_middle(middle),
    m_right(right),
    m_size((left  ? left->size()  : 0) +
           (middle? 1             : 0) +
           (right ? right->size() : 0))
{
  checkSizes();
}


FunctionalSet::~FunctionalSet()
{}


FSElement const *FunctionalSet::at(size_type index) const
{
  xassert(0 <= index && index < size());

  size_type middleIndex = leftSizeForTotal(m_size);

  if (index < middleIndex) {
    xassert(m_left);
    return m_left->at(index);
  }
  else if (index == middleIndex) {
    xassert(m_middle);
    return m_middle;
  }
  else {
    xassert(m_right);
    return m_right->at(index - (middleIndex+1));
  }
}


bool FunctionalSet::contains(FSElement const *elt) const
{
  if (empty()) {
    return false;
  }

  xassert(m_middle);
  StrongOrdering ord = elt->compareTo(*m_middle);

  if (ord == StrongOrdering::less) {
    if (m_left) {
      return m_left->contains(elt);
    }
    else {
      return false;
    }
  }
  else if (ord == StrongOrdering::equal) {
    return true;
  }
  else {
    if (m_right) {
      return m_right->contains(elt);
    }
    else {
      return false;
    }
  }
}


void FunctionalSet::getElements(std::vector<RCPtr<FSElement> > &vec) const
{
  vec.reserve(size());

  if (m_left) {
    m_left->getElements(vec);
  }
  if (m_middle) {
    vec.push_back(m_middle);
  }
  if (m_right) {
    m_right->getElements(vec);
  }
}


void FunctionalSet::checkInvariants() const
{
  checkSizes();

  // This is linear in the size of the set.
  checkBounds(NULL, NULL);
}


void FunctionalSet::checkBounds(
  FSElement const * /*nullable*/ lowBound,
  FSElement const * /*nullable*/ highBound) const
{
  if (m_middle) {
    if (lowBound) {
      xassert(lowBound->compareTo(*m_middle) < 0);
    }
    if (highBound) {
      xassert(m_middle->compareTo(*highBound) < 0);
    }
  }

  if (m_left) {
    m_left->checkBounds(lowBound, m_middle);
  }
  if (m_right) {
    m_right->checkBounds(m_middle, highBound);
  }
}


void FunctionalSet::checkSizes() const
{
  if (m_left) {
    xassert(m_left->size() == leftSizeForTotal(m_size));
  }
  if (m_right) {
    xassert(m_right->size() == rightSizeForTotal(m_size));
  }
}


char const *FunctionalSet::fseKind() const
{
  return "FunctionalSet";
}


StrongOrdering FunctionalSet::compareTo(FSElement const &obj_) const
{
  FSELEMENT_COMPARETO_PRELUDE(FunctionalSet);

  // For now, use the naive algorithm: get all elements, then compare
  // them in order.

  std::vector<RCPtr<FSElement> > avec;
  std::vector<RCPtr<FSElement> > bvec;

  this->getElements(avec);
  obj.getElements(bvec);

  size_type ai = 0;
  size_type bi = 0;
  while (ai < avec.size() && bi < bvec.size()) {
    StrongOrdering ord = avec[ai]->compareTo(*(bvec[bi]));
    if (ord != StrongOrdering::equal) {
      return ord;
    }
    ++ai;
    ++bi;
  }

  if (bi < bvec.size()) {
    // 'a' (meaning 'this') ended first, so it is less.
    return StrongOrdering::less;
  }
  else if (ai < avec.size()) {
    return StrongOrdering::greater;
  }
  else {
    return StrongOrdering::equal;
  }
}


void FunctionalSet::print(std::ostream &os) const
{
  if (empty()) {
    os << "{}";
  }
  else {
    os << "{ ";
    printElements(os);
    os << " }";
  }
}


void FunctionalSet::printElements(std::ostream &os) const
{
  if (m_left) {
    m_left->printElements(os);
    os << ", ";
  }

  if (m_middle) {
    m_middle->print(os);
  }
  else {
    // We expect 'm_middle' to be non-NULL, but since we're in a print
    // routine, which is sometimes used in a debug context where
    // invariants might not hold, be tolerant of a NULL element.
    os << "(NULL middle element)";
  }

  if (m_right) {
    os << ", ";
    m_right->printElements(os);
  }
}


// ---------------------- FunctionalSetManager -------------------------
FunctionalSetManager::FunctionalSetManager()
  : m_sets()
{}


FunctionalSetManager::~FunctionalSetManager()
{}


RCPtr<FunctionalSet> FunctionalSetManager::setFromVector(
  std::vector<RCPtr<FSElement> > const &vec)
{
  return setFromVectorRange(vec, 0, vec.size());
}


RCPtr<FunctionalSet> FunctionalSetManager::nullableSetFromVectorRange(
  std::vector<RCPtr<FSElement> > const &vec,
  std::size_t start, std::size_t end)
{
  if (start == end) {
    return RCPtr<FunctionalSet>(NULL);
  }
  else {
    return setFromVectorRange(vec, start, end);
  }
}


RCPtr<FunctionalSet> FunctionalSetManager::setFromVectorRange(
  std::vector<RCPtr<FSElement> > const &vec,
  std::size_t start, std::size_t end)
{
  // Candidate new set for insertion.
  RCPtr<FunctionalSet> newSet;

  // Build the candidate set.
  size_type n = end-start;
  if (n == 0) {
    newSet = new FunctionalSet(
      RCPtr<FunctionalSet>(NULL),      // left
      RCPtr<FSElement>(NULL),          // middle
      RCPtr<FunctionalSet>(NULL)       // right
    );
  }
  else {
    size_type leftSize = FunctionalSet::leftSizeForTotal(n);
    size_type rightSize = FunctionalSet::rightSizeForTotal(n);
    newSet = new FunctionalSet(
      nullableSetFromVectorRange(vec, 0, leftSize),
      vec[start+leftSize],
      nullableSetFromVectorRange(vec, leftSize+1, leftSize+1+rightSize)
    );
  }

  // See if we already have it.
  auto it = m_sets.insert(newSet);
  if (!it.second) {
    // Use the existing value instead.
    newSet = *(it.first);
  }

  return newSet;
}


RCPtr<FunctionalSet> FunctionalSetManager::emptySet()
{
  std::vector<RCPtr<FSElement> > vec;
  return setFromVector(vec);
}


RCPtr<FunctionalSet> FunctionalSetManager::singleton(RCPtr<FSElement> elt)
{
  std::vector<RCPtr<FSElement> > vec;
  vec.push_back(elt);
  return setFromVector(vec);
}


RCPtr<FunctionalSet> FunctionalSetManager::unionSet(
  RCPtr<FunctionalSet> a, RCPtr<FunctionalSet> b)
{
  std::vector<RCPtr<FSElement> > avec;
  std::vector<RCPtr<FSElement> > bvec;

  a->getElements(avec);
  b->getElements(bvec);

  // Construct 'vec' by merging 'avec' and 'bvec'.
  std::vector<RCPtr<FSElement> > vec;

  size_type ai = 0;
  size_type bi = 0;
  while (ai < avec.size() && bi < bvec.size()) {
    StrongOrdering ord = avec[ai]->compareTo(*(bvec[bi]));
    if (ord == StrongOrdering::less) {
      // Take the element from 'a'.
      vec.push_back(avec[ai]);
      ++ai;
    }
    else if (ord == StrongOrdering::greater) {
      // Take from 'b'.
      vec.push_back(bvec[bi]);
      ++bi;
    }
    else {
      // Take from either (they are equal) and advance both.
      vec.push_back(avec[ai]);
      ++ai;
      ++bi;
    }
  }

  // Append whatever was left over.
  while (ai < avec.size()) {
    vec.push_back(avec[ai++]);
  }
  while (bi < bvec.size()) {
    vec.push_back(bvec[bi++]);
  }

  return setFromVector(vec);
}


RCPtr<FunctionalSet> FunctionalSetManager::intersection(
  RCPtr<FunctionalSet> a, RCPtr<FunctionalSet> b)
{
  std::vector<RCPtr<FSElement> > avec;
  std::vector<RCPtr<FSElement> > bvec;

  a->getElements(avec);
  b->getElements(bvec);

  // Construct 'vec' by merging 'avec' and 'bvec'.
  std::vector<RCPtr<FSElement> > vec;

  size_type ai = 0;
  size_type bi = 0;
  while (ai < avec.size() && bi < bvec.size()) {
    StrongOrdering ord = avec[ai]->compareTo(*(bvec[bi]));
    if (ord == StrongOrdering::less) {
      // Skip element from 'a'.
      ++ai;
    }
    else if (ord == StrongOrdering::greater) {
      // Skip from 'b'.
      ++bi;
    }
    else {
      // Take from either (they are equal) and advance both.
      vec.push_back(avec[ai]);
      ++ai;
      ++bi;
    }
  }

  // Ignore whatever was left over.

  return setFromVector(vec);
}


void FunctionalSetManager::checkInvariants() const
{
  // Each iteration is linear in the size of its respective set (both
  // the comparison and 'checkInvariants' are linear), so the entire
  // procedure is worst-case quadratic.

  RCPtr<FunctionalSet> prev(NULL);
  for (auto p : m_sets) {
    // Make sure the order is right.
    if (prev) {
      xassert(prev->compareTo(*p) == StrongOrdering::less);
    }
    prev = p;

    p->checkInvariants();
  }
}


// EOF
