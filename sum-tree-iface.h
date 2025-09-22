// sum-tree-iface.h
// Interface for `sum-tree` module.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_SUM_TREE_IFACE_H
#define SMBASE_SUM_TREE_IFACE_H

#include "sum-tree-fwd.h"              // fwds for this module

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [n]
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-utility-fwd.h"    // std::pair [n]
#include "smbase/std-vector-fwd.h"     // stdfwd::vector [n]

#include <memory>                      // std::unique_ptr


OPEN_NAMESPACE(smbase)


/* A `SumTree<T>` is a *sequence* of T that can be indexed using a
   property that acts as the "summary" of T.

   For example, if T is a string, and we use its size as the summary,
   then the tree represents the virtual concatentation of all of its
   elements and can be efficiently accessed using an offset into that
   virtual concatenation.

   It is implemented using a balanced binary tree, thus affording
   logarithmic lookup by the summary property.

   T must have a type `T::Summary` and a method `summary()` that
   computes it.

   `T::Summary` must be default-constructible to yield the zero element,
   addable and subtractable using `+` and `-`, and comparable with `<`,
   `<=`, and `==`.

   TODO: Split "Summary" as a collection of summarizable attributes from
   "Summary" as one attribute that can be used as an index.
*/
template <typename T>
class SumTree {
public:      // types
  using Summary = typename T::Summary;

  // Result of lookup: a reference to an element, along with the amount
  // of the summary that was not accounted for by elements earlier in
  // the sequence.
  using LookupResult = std::pair<T const &, Summary>;

private:     // types
  // Forward, defined a bit later in this class.
  class InteriorNode;

  class Node {
  public:      // data
    // Summary of this subtree.
    //
    // For a leaf, this is `m_data.summary()`.
    //
    // For an interior node, this is the sum of the child summaries.
    Summary m_summary;

    // Height of the subtree rooted here.  0 if this is a leaf.
    // Otherwise, it is one more than the maximum of the child heights.
    int m_height;

  public:      // methods
    virtual ~Node();

    Node(Summary summary, int height);

    // Assert invariants, including in subtrees.
    virtual void selfCheck() const = 0;

    // Return `this` as an `InteriorNode*`.  Requires that it is one.
    virtual InteriorNode *asInteriorNode() = 0;

    // Recursive lookup starting at `*this`.
    virtual LookupResult lookup(Summary s) const = 0;

    // Node balance factor: left child height - right child height.
    //
    // Positive means *left* is heavy.
    //
    // Negative means *right* is heavy.
    //
    // This is zero for any leaf node.
    virtual int balanceFactor() const = 0;

    // Append to `dest` the sequence of all elements in the subtree
    // rooted at `this`, along with the summary of all preceding nodes,
    // given that the nodes preceding this subtree are summarized by
    // `s`.
    //
    // Return the combined summary of `s` and the nodes here.
    virtual Summary getAllElements(
      stdfwd::vector<LookupResult> &dest /*APPEND*/,
      Summary s) const = 0;

    // Dump internal tree.
    virtual operator gdv::GDValue() const = 0;

    // Add to `m` the members declared in `Node`.
    void writeNodeMembers(gdv::GDValue &m) const;

    // Insert `t` into the subtree rooted at `this`.
    //
    // When this method is invoked, `this` is *detached* from the tree;
    // it is effectively its own owner/unique pointer.
    //
    // Return a unique pointer to the augmented tree, which must either
    // be `this` itself or a new node owns `this` either directly or
    // indirectly.
    virtual std::unique_ptr<Node> append(T const &t) = 0;
  };

  using NodeUPtr = std::unique_ptr<Node>;

  class InteriorNode : public Node {
  public:      // data
    // Child subtrees.  Never null.
    //
    // Invariant: The AVL balance condition, i.e., that their heights do
    // not differ by more than one.
    NodeUPtr m_left;
    NodeUPtr m_right;

  private:     // methods
    // Rotate as needed to balance this node.
    void balance();

    // See comments at implementation.
    void rotateLeft();
    void rotateRight();

    // Rebuild this node with new children, such that the invariants are
    // established with those new children.
    //
    // Requires: m_left == m_right == nullptr
    void rebuild(NodeUPtr left, NodeUPtr right);

    // Recompute local data after changing child pointers.  The node
    // invariants might not hold before or after.
    void localRecompute_brokenInvariants();

  public:      // methods
    virtual ~InteriorNode() override;

    // Computes the summary and height from the children.
    InteriorNode(NodeUPtr left, NodeUPtr right);

    // Assert local invariants only.
    void localSelfCheck() const;

    // Node method overrides.
    virtual void selfCheck() const override;
    virtual InteriorNode *asInteriorNode() override;
    virtual LookupResult lookup(Summary s) const override;
    virtual int balanceFactor() const override;
    virtual Summary getAllElements(
      stdfwd::vector<LookupResult> &dest /*APPEND*/,
      Summary s) const override;
    virtual operator gdv::GDValue() const override;
    virtual NodeUPtr append(T const &t) override;
  };

  class Leaf : public Node {
  public:      // data
    // Data stored in this leaf node.
    T m_data;

  public:      // methods
    virtual ~Leaf() override;

    // Computes the summary from the provided data.
    Leaf(T const &data);

    // Node method overrides.
    virtual void selfCheck() const override;
    virtual InteriorNode *asInteriorNode() override;
    virtual LookupResult lookup(Summary s) const override;
    virtual int balanceFactor() const override;
    virtual Summary getAllElements(
      stdfwd::vector<LookupResult> &dest /*APPEND*/,
      Summary s) const override;
    virtual operator gdv::GDValue() const override;
    virtual NodeUPtr append(T const &t) override;
  };

private:     // data
  // Root of the tree, or null if it is empty.
  NodeUPtr m_root;

public:      // methods
  ~SumTree();

  // Empty tree.
  SumTree();

  // Assert invariants.
  void selfCheck() const;

  // ----------------------------- Queries -----------------------------
  // Summary of the entire tree.
  Summary summary() const;

  // Find the first element for which the summary of all preceding
  // elements is less than or equal to `s`.  The returned summary is
  // `s` minus the summary of all preceding elements.
  //
  // Requires: Summary() <= s < summary()
  LookupResult lookup(Summary s) const;

  // Return the sequence of elements, and for each, the summary of all
  // preceding elements.
  stdfwd::vector<LookupResult> allElements() const;

  // Dump internal tree.  Requires that `toGDValue(T)` exist.
  operator gdv::GDValue() const;

  // -------------------------- Modifications --------------------------
  // Reset to an empty sequence.
  void clear();

  // Add `t` to the end of the sequence.
  void append(T const &t);
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SUM_TREE_IFACE_H
