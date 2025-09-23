// sum-tree.h
// `SumTree`, a balanced tree with summary-based indexing.

// See license.txt for copyright and terms of use.

// The approach here is most directly based on the description at:
//
//   https://zed.dev/blog/zed-decoded-rope-sumtree
//
// although this is an idea I've worked with previously.

#ifndef SMBASE_SUM_TREE_H
#define SMBASE_SUM_TREE_H

#include "sum-tree-iface.h"            // interface for this module

#include "smbase/chained-cond.h"       // smbase::cc::le_le
#include "smbase/gdvalue-unique-ptr.h" // gdv::toGDValue(std::unique_ptr)
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/get-type-name.h"      // smbase::GetTypeName
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/xassert.h"            // xassert, smbase::xassertPtr

#include <algorithm>                   // std::max
#include <memory>                      // std::unique_ptr
#include <utility>                     // std::{move,pair}
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// ------------------------------- Node --------------------------------
template <typename T>
SumTree<T>::Node::~Node()
{}


template <typename T>
SumTree<T>::Node::Node(Summary summary, int height)
:
  m_summary(summary),
  m_height(height)
{}


template <typename T>
void SumTree<T>::Node::writeNodeMembers(gdv::GDValue &m) const
{
  using namespace gdv;

  GDV_WRITE_MEMBER_SYM(m_summary);
  GDV_WRITE_MEMBER_SYM(m_height);
}


// --------------------------- InteriorNode ----------------------------
template <typename T>
SumTree<T>::InteriorNode::~InteriorNode()
{}


template <typename T>
SumTree<T>::InteriorNode::InteriorNode(NodeUPtr left, NodeUPtr right)
:
  Node(left->m_summary + right->m_summary,
       1 + std::max(left->m_height, right->m_height)),
  m_size(left->size() + right->size()),
  m_left(std::move(left)),
  m_right(std::move(right))
{
  localSelfCheck();
}


template <typename T>
void SumTree<T>::InteriorNode::localSelfCheck() const
{
  xassert(this->m_summary == m_left->m_summary + m_right->m_summary);
  xassert(this->m_size == m_left->size() + m_right->size());
  xassert(cc::le_le(-1, balanceFactor(), +1));
}


template <typename T>
void SumTree<T>::InteriorNode::selfCheck() const
{
  xassert(m_left != nullptr);
  xassert(m_right != nullptr);

  localSelfCheck();

  m_left->selfCheck();
  m_right->selfCheck();
}


template <typename T>
auto SumTree<T>::InteriorNode::size() const -> size_type
{
  return m_size;
}


template <typename T>
T const &SumTree<T>::InteriorNode::atC(size_type index) const
{
  xassertPrecondition(cc::z_le_lt(index, m_size));

  auto leftSize = m_left->size();
  if (index < leftSize) {
    return m_left->atC(index);
  }
  else {
    return m_right->atC(index - leftSize);
  }
}


template <typename T>
auto SumTree<T>::InteriorNode::asInteriorNode() -> InteriorNode *
{
  return this;
}


// See diagram `sum-tree-balance.ded.png` for an explanation of the
// logic of this function.
template <typename T>
void SumTree<T>::InteriorNode::balance()
{
  int const bf = this->balanceFactor();

  if (bf > 1) {
    if (m_left->balanceFactor() < 0) {
      // The child must be an interior node because only an interior
      // node can have a non-zero balance factor.
      m_left->asInteriorNode()->rotateLeft();
    }
    this->rotateRight();
  }

  else if (bf < -1) {
    if (m_right->balanceFactor() > 0) {
      m_right->asInteriorNode()->rotateRight();
    }
    this->rotateLeft();
  }

  localSelfCheck();
}


/* Rotate node right:

         this               this (rebuilt)
        /    \             /    \
       b      d    ==>    a      b (rebuilt)
      / \                       / \
     a   c                     c   d
*/
template <typename T>
void SumTree<T>::InteriorNode::rotateRight()
{
  NodeUPtr b = std::move(m_left);
  InteriorNode *bi = b->asInteriorNode();

  NodeUPtr a = std::move(bi->m_left);
  NodeUPtr c = std::move(bi->m_right);
  NodeUPtr d = std::move(m_right);

  bi->rebuild(std::move(c), std::move(d));
  this->rebuild(std::move(a), std::move(b));
}


/* Rotate node left:

         this               this (rebuilt)
        /    \             /    \
       a      c    ==>    c (r)  d
             / \         / \
            b   d       a   b
*/
template <typename T>
void SumTree<T>::InteriorNode::rotateLeft()
{
  NodeUPtr c = std::move(m_right);
  InteriorNode *ci = c->asInteriorNode();

  NodeUPtr b = std::move(ci->m_left);
  NodeUPtr d = std::move(ci->m_right);
  NodeUPtr a = std::move(m_left);

  ci->rebuild(std::move(a), std::move(b));
  this->rebuild(std::move(c), std::move(d));
}


template <typename T>
void SumTree<T>::InteriorNode::rebuild(NodeUPtr left, NodeUPtr right)
{
  xassertPrecondition(m_left == nullptr);
  xassertPrecondition(m_right == nullptr);

  m_left = std::move(left);
  m_right = std::move(right);

  // Here, recomputation should suffice to restore invariants.
  localRecompute_brokenInvariants();
  localSelfCheck();
}


template <typename T>
void SumTree<T>::InteriorNode::localRecompute_brokenInvariants()
{
  // Node members need `this->` due to template lookup rules.
  this->m_summary = m_left->m_summary + m_right->m_summary;
  this->m_height = 1 + std::max(m_left->m_height, m_right->m_height);

  m_size = m_left->size() + m_right->size();
}


template <typename T>
auto SumTree<T>::InteriorNode::lookup(Summary s) const -> LookupResult
{
  if (s < m_left->m_summary) {
    return m_left->lookup(s);
  }
  else {
    return m_right->lookup(s - m_left->m_summary);
  }
}


template <typename T>
int SumTree<T>::InteriorNode::balanceFactor() const
{
  return m_left->m_height - m_right->m_height;
}


template <typename T>
auto SumTree<T>::InteriorNode::getAllElements(
  std::vector<LookupResult> &dest /*APPEND*/,
  Summary s) const
  -> Summary
{
  s = m_left->getAllElements(dest, s);
  s = m_right->getAllElements(dest, s);
  return s;
}


template <typename T>
SumTree<T>::InteriorNode::operator gdv::GDValue() const
{
  using namespace gdv;

  GDValue m(GDVK_TAGGED_ORDERED_MAP, "InteriorNode"_sym);
  this->writeNodeMembers(m);
  m.mapSetValueAtSym("balanceFactor", balanceFactor());
  GDV_WRITE_MEMBER_SYM(m_size);
  GDV_WRITE_MEMBER_SYM(m_left);
  GDV_WRITE_MEMBER_SYM(m_right);

  return m;
}


template <typename T>
auto SumTree<T>::InteriorNode::append(T const &t) -> NodeUPtr
{
  // Appends always go into right subtree.
  m_right = m_right.release()->append(t);

  // The new right child might make this node unbalanced, but we want to
  // recompute the summary, since if this node *is* still balanced, then
  // `balance()` won't make any further changes.
  localRecompute_brokenInvariants();

  balance();

  return NodeUPtr(this);
}


// ------------------------------- Leaf --------------------------------
template <typename T>
SumTree<T>::Leaf::~Leaf()
{}


template <typename T>
SumTree<T>::Leaf::Leaf(T const &data)
:
  Node(data.summary(), 0),
  m_data(data)
{}


template <typename T>
void SumTree<T>::Leaf::selfCheck() const
{
  xassert(this->m_summary == m_data.summary());
  xassert(this->m_height == 0);
}


template <typename T>
auto SumTree<T>::Leaf::size() const -> size_type
{
  return 1;
}


template <typename T>
T const &SumTree<T>::Leaf::atC(size_type index) const
{
  xassertPrecondition(index == 0);
  return m_data;
}


template <typename T>
auto SumTree<T>::Leaf::asInteriorNode() -> InteriorNode *
{
  xfailure("Tried to treat a Leaf as an InteriorNode.");
  return nullptr;   // Not reached.
}


template <typename T>
auto SumTree<T>::Leaf::lookup(Summary s) const -> LookupResult
{
  return LookupResult(m_data, s);
}


template <typename T>
int SumTree<T>::Leaf::balanceFactor() const
{
  return 0;
}


template <typename T>
auto SumTree<T>::Leaf::getAllElements(
  std::vector<LookupResult> &dest /*APPEND*/,
  Summary s) const
  -> Summary
{
  dest.push_back({m_data, s});
  return s + m_data.summary();
}


template <typename T>
SumTree<T>::Leaf::operator gdv::GDValue() const
{
  using namespace gdv;

  GDValue m(GDVK_TAGGED_ORDERED_MAP, "Leaf"_sym);
  this->writeNodeMembers(m);
  GDV_WRITE_MEMBER_SYM(m_data);

  return m;
}


template <typename T>
auto SumTree<T>::Leaf::append(T const &t) -> NodeUPtr
{
  return std::make_unique<InteriorNode>(
    NodeUPtr(this),
    std::make_unique<Leaf>(t));
}


// ------------------------------ SumTree ------------------------------
template <typename T>
SumTree<T>::~SumTree()
{}


template <typename T>
SumTree<T>::SumTree()
  : m_root(nullptr)
{}


template <typename T>
void SumTree<T>::selfCheck() const
{
  if (m_root) {
    m_root->selfCheck();
  }
}


// -------------------------- SumTree Queries --------------------------
template <typename T>
auto SumTree<T>::size() const -> size_type
{
  return m_root? m_root->size() : 0;
}


template <typename T>
T const &SumTree<T>::atC(size_type index) const
{
  xassertPrecondition(cc::z_le_lt(index, size()));
  return m_root->atC(index);
}


template <typename T>
typename SumTree<T>::Summary SumTree<T>::summary() const
{
  return m_root? m_root->m_summary : Summary();
}


template <typename T>
auto SumTree<T>::lookup(Summary s) const -> LookupResult
{
  xassert(m_root);
  xassert(Summary() <= s && s < m_root->m_summary);
  return m_root->lookup(s);
}


template <typename T>
auto SumTree<T>::allElements() const -> std::vector<LookupResult>
{
  std::vector<LookupResult> ret;

  if (m_root) {
    m_root->getAllElements(ret, Summary());
  }

  return ret;
}


template <typename T>
SumTree<T>::operator gdv::GDValue() const
{
  using namespace gdv;

  GDValue m(GDVK_TAGGED_ORDERED_MAP, "SumTree"_sym);

  m.orderedMapSetValueAtSym("T",
    GDVSymbol(GetTypeName<T>::name()));
  GDV_WRITE_MEMBER_SYM(m_root);

  return m;
}


// ----------------------- SumTree Modifications -----------------------
template <typename T>
void SumTree<T>::clear()
{
  m_root.reset();
}


template <typename T>
void SumTree<T>::append(T const &t)
{
  if (!m_root) {
    m_root = std::make_unique<Leaf>(t);
  }
  else {
    m_root = m_root.release()->append(t);
  }
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SUM_TREE_H
