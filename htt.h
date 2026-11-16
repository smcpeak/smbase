// htt.h
// Hierarchical Task Timers.

// This file is in the public domain.

#ifndef SMBASE_HTT_H
#define SMBASE_HTT_H

#include "htt-fwd.h"                             // fwds for this module

#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue [f]
#include "smbase/indexed-string-table.h"         // smbase::IndexedStringTable
#include "smbase/ordered-mru-flat-map-iface.h"   // smbase::OrderedMRUFlatMap [i]
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/std-string-fwd.h"               // std::string [f]
#include "smbase/std-string-view-fwd.h"          // std::string_view [f]

#include <memory>                                // std::unique_ptr


OPEN_NAMESPACE(smbase)


// Type of an integer index that identifies a label string.  The table
// that generates the indices is stored inside the relevant HTT_Tree.
typedef IndexedStringTable::Index HTT_LabelIndex;


// One node in the tree of tasks.  It holds the accumulated elapsed
// time, and its position in the tree indicates the task's identity as a
// sequence of labels (those on the path from the root to this node)
// that correspond to nested activities, similar to a call stack.
class HTT_Node {
private:     // types
  using LabelIndexToChildMap =
    OrderedMRUFlatMap<HTT_LabelIndex, std::unique_ptr<HTT_Node>>;

private:     // data
  // Parent of this node.  Having this pointer means we can easily pop
  // elements from the node stack without keeping them in a vector.
  //
  // Invariant: `m_parent==nullptr` iff this is the root node.
  HTT_Node * NULLABLE m_parent;

  // Map from HTT_LabelIndex to child HTT_Node.
  //
  // Invariant: No value is nullptr.
  LabelIndexToChildMap m_children;

  // Elapsed time, in milliseconds, accumulated while this node was on
  // the stack.
  unsigned m_elapsedMS;

public:      // funcs
  ~HTT_Node();

  // Create a node with the specified parent, no children, and zero
  // elapsed time.
  HTT_Node(HTT_Node * NULLABLE parent);

  // Get or create a child for `labelIndex`.
  //
  // The returned object is deallocated when the entire tree is.
  //
  // Ensures: return->m_parent == this
  HTT_Node *getChild(HTT_LabelIndex labelIndex);

  void addElapsedMS(unsigned ms)
    { m_elapsedMS += ms; }

  HTT_Node * NULLABLE getParent()
    { return m_parent; }

  /*
    Return as an untagged ordered map with keys:

    * elapsedMS: Integer m_elapsedMS.

    * children: Ordered map from child label (as a string) to its GDV.

    Example:

    [
      elapsedMS: 123
      children: [
        "child1": [...]
        "child2": [...]
      ]
    ]
  */
  gdv::GDValue asGDValue(HTT_Tree const &tree) const;
};


// Tree of HTT nodes.  Normally there is just one in the process for
// production use, but additional trees can be created for testing.
class HTT_Tree {
private:     // data
  // Map between labels and their indices.
  IndexedStringTable m_labelTable;

  // Root tree node.
  HTT_Node m_root;

  // Top-most (innermost) active node.  Initially `&m_root`.
  //
  // Invariant: m_stackTop != nullptr
  HTT_Node *m_stackTop;

public:      // funcs
  ~HTT_Tree();

  // Make a tree that is empty except for its root.
  HTT_Tree();

  // Return a numeric index for `label`.
  HTT_LabelIndex getIndexForLabel(std::string_view label);

  // Requires: `index` is valid, having been previously returned by
  // `getIndexForLabel`.
  std::string_view getLabelForIndex(HTT_LabelIndex index) const;

  // Get or create a node that is a child of the current top of stack
  // node, but do not push it.
  //
  // Ensures: return->m_parent == m_stackTop
  HTT_Node *getTOSChildNode(HTT_LabelIndex labelIndex);

  // Push a node that is not currently on the stack onto its top.
  //
  // Requires: newStackTop->m_parent == m_stackTop
  //
  // Ensures: m_stackTop == newStackTop
  void pushNode(HTT_Node *newStackTop);

  // Pop the top of stack.
  //
  // Requires: oldStackTop == m_stackTop
  //
  // Ensures: m_stackTop == oldStackTop->m_parent
  void popNode(HTT_Node *oldStackTop);

  /*
    Return the root's GDV, for example:

    [
      elapsedMS: 456
      children: [
        "one": [
          elapsedMS: 123
          children: [
            "two": [
              elapsedMS: 2
              children: [
                "three": [
                  elapsedMS: 3
                  children: [:]
                ]
              ]
            ]
          ]
        ]
        "two": [
          elapsedMS: 4
          children: [
            "three": [
              elapsedMS: 5
              children: [:]
            ]
          ]
        ]
        "three": [
          elapsedMS: 6
          children: [:]
        ]
      ]
    ]
  */
  operator gdv::GDValue() const;

  // Render as indented GDVN.
  std::string asString() const;
};


// Start a node timer on entry.  Stop and accumulate on exit.
class HTT_StartStopNode {
private:     // data
  // Associated node in the HTT tree.
  HTT_Node *m_node;

  // Timer value when we start.
  unsigned m_startMS;

public:      // funcs
  // Save `node`, start the timer.
  HTT_StartStopNode(HTT_Node *node);

  // Add elapsed time to `node`.
  ~HTT_StartStopNode();

  HTT_Node *getNode() const
    { return m_node; }

  // If we were to stop the timer now, how many milliseconds would get
  // added to `m_node`?
  unsigned elapsedMSSoFar() const;
};


// Push and start a node timer identified by a tree and label on entry.
// Stop, accumulate, and pop on exit.
class HTT_StartStopLabel : public HTT_StartStopNode {
private:     // data
  // The tree holding the label table and the timer nodes.
  HTT_Tree &m_tree;

public:      // funcs
  // Use `pushNode` to get and push a node, and start its timer.
  HTT_StartStopLabel(HTT_Tree &tree, HTT_LabelIndex labelIndex);

  // Stop the timer, pop the node.
  ~HTT_StartStopLabel();
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_HTT_H
