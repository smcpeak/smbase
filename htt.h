// htt.h
// Hierarchical Task Timers.

// This file is in the public domain.

#ifndef SMBASE_HTT_H
#define SMBASE_HTT_H

#include "htt-fwd.h"                             // fwds for this module

#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue [f]
#include "smbase/indexed-string-table.h"         // smbase::IndexedStringTable
#include "smbase/ordered-mru-flat-map-iface.h"   // smbase::OrderedMRUFlatMap [i]
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, NULLABLE, NO_OBJECT_COPIES
#include "smbase/sm-pp-util.h"                   // SM_PP_CAT
#include "smbase/std-set-fwd.h"                  // stdfwd::set [f]
#include "smbase/std-string-fwd.h"               // std::string [f]
#include "smbase/std-string-view-fwd.h"          // std::string_view [f]
#include "smbase/text-table-fwd.h"               // smbase::TextTable [f]

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
  //
  // Invariant: For all valid i:
  //   m_children[i]->m_parent == this
  //
  LabelIndexToChildMap m_children;

  // True if this timer is running.
  bool m_running;

  // If `m_running`, this is when it started.  Otherwise it is
  // meaningless.
  unsigned m_startMS;

  // Elapsed time, in milliseconds, accumulated while this node was on
  // the stack.
  //
  // Exception: The root node always has zero elapsed time because we
  // never start/stop it; it is just a container, and is not associated
  // with any particular task.
  unsigned m_elapsedMS;

public:      // funcs
  ~HTT_Node();

  // Recursively assert invariants.  `stackNodes` is the set of nodes
  // on the containing tree's stack.
  void selfCheck(stdfwd::set<HTT_Node const *> const &stackNodes) const;

  // Create a node with the specified parent, no children, and zero
  // elapsed time.
  explicit HTT_Node(HTT_Node * NULLABLE parent);

  HTT_Node * NULLABLE getParent() const
    { return m_parent; }

  // Get or create a child for `labelIndex`.
  //
  // The returned object is deallocated when the entire tree is.
  //
  // Ensures: return->m_parent == this
  HTT_Node *getChild(HTT_LabelIndex labelIndex);

  bool running() const
    { return m_running; }

  // Start this timer.
  //
  // Requires: running() == false
  //
  // Ensures: running() == true
  void start();

  // Stop this timer.
  //
  // Requires: running() == true
  //
  // Ensures: running() == false
  void stop();

  // Elapsed time in `m_elapsedMS`, plus, if `running()`,
  // `currentMS - m_startMS`.
  unsigned elapsedPlusRunningMS(unsigned currentMS) const;

  /*
    Return as an untagged ordered map, starting with the scalars in the
    obvious way, then the children as an ordered map from child label
    (as a string) to its GDV.

    Example:

    [
      running: true
      startMS: 45
      elapsedMS: 123
      children: [
        "child1": [...]
        "child2": [...]
      ]
    ]
  */
  gdv::GDValue asGDValue(HTT_Tree const &tree) const;

  // Recursively add to `tt.m_cells` rows for this node's children.
  void renderTableRows(
    TextTable &tt,                // Destination table to build.
    HTT_Tree const &tree,         // Used to decode label indices.
    std::string_view label,       // This node's label text.
    int indent,                   // How much to indent the label.
    unsigned currentMS) const;    // Current clock value.
};


// Tree of HTT nodes.  Normally there is just one in the process for
// production use, but additional trees can be created for testing.
class HTT_Tree {
private:     // data
  // Map between labels and their indices.
  IndexedStringTable m_labelTable;

  // Root tree node.
  //
  // Invariant: m_root.m_running == true
  HTT_Node m_root;

  // Top-most (innermost) active node.  Initially `&m_root`.
  //
  // Invariant: m_stackTop != nullptr
  //
  // Invariant: Every node that is reachable from `m_stackTop` by
  // traversing `m_parent` pointers, including `m_stackTop` itself, is
  // running; and all other nodes are not running.
  //
  HTT_Node *m_stackTop;

public:      // funcs
  ~HTT_Tree();

  // Assert invariants throughout tree.
  void selfCheck() const;

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

  // Push a node that is not currently on the stack onto its top and
  // start its timer.
  //
  // Requires: newStackTop->getParent() == m_stackTop
  // Requires: newStackTop->running() == false
  //
  // Ensures: m_stackTop == newStackTop
  // Ensures: newStackTop->running() == true
  void pushNode(HTT_Node *newStackTop);

  // Pop the top of stack and stop its timer.
  //
  // Requires: oldStackTop == m_stackTop
  // Requires: oldStackTop->running() == true
  //
  // Ensures: m_stackTop == oldStackTop->getParent()
  // Ensures: oldStackTop->running() == false
  void popNode(HTT_Node *oldStackTop);

  // Return the root's GDV.
  operator gdv::GDValue() const;

  // Render as indented GDVN.
  std::string asString() const;

  /*
    Render as a plain text table like this:

      timer        elapsed_ms  r
      -----------  ----------  -
      root                234  r
        one               123  r
          two               2  r
            three           3  r
        two                 4
          three             5
        three               6

    The elapsed time is as reported by `HTTNode::elapsed()` for the
    current time.

    The "r" column is "r" if that timer is running.

    If `fasttime` is not implemented on this platform, add a warning to
    that effect as a final line.
  */
  std::string asTableString() const;
};


// Start a node timer on entry.  Stop and accumulate on exit.
class HTT_StartStopNode {
private:     // data
  // The tree holding the label table and the timer nodes.
  HTT_Tree &m_tree;

  // Associated node in the HTT tree.
  HTT_Node *m_node;

public:      // funcs
  // Save `node`, start the timer.
  explicit HTT_StartStopNode(HTT_Tree &tree, HTT_Node *node);

  // Stop the timer.
  ~HTT_StartStopNode();

  HTT_Node *getNode() const
    { return m_node; }
};


// Start and stop, using the label to identify the node.
class HTT_StartStopLabel : public HTT_StartStopNode {
public:      // funcs
  // Start.
  explicit HTT_StartStopLabel(HTT_Tree &tree, HTT_LabelIndex labelIndex);

  // Stop.
  ~HTT_StartStopLabel();
};


/* Object that establishes itself as the unique global singleton tree
   that the HTT macros interact with.

   Only one such object can be created during the process lifetime
   because the macros use `static` for efficiency to avoid repeated
   lookups, and there is no way to "reset" the state of a `static` local
   variable.
*/
class HTT_Singleton {
  NO_OBJECT_COPIES(HTT_Singleton);

private:     // class data
  // Current singleton instance, or nullptr if there is none.  Initially
  // nullptr.  The singleton is created on demand by `getIndexForLabel`,
  // and never destroyed.
  static HTT_Singleton * NULLABLE s_instance;

private:     // instance data
  // The tree data this object wraps.
  HTT_Tree m_tree;

private:     // funcs
  // This is private because creation is done by `getIndexForLabel`.
  HTT_Singleton();

public:      // funcs
  // Create the instance if it has not been.
  //
  // Ensures: s_instance != nullptr
  static void ensureCreated();

  // Map label text to an index.
  //
  // This also creates the instance if needed.
  //
  // Ensures: s_instance != nullptr
  static HTT_LabelIndex getIndexForLabel(std::string_view label);

  // Return the instance.
  //
  // Requires: s_instance != nullptr
  static HTT_Singleton &instance()
    { return *s_instance; }

  // Return the global tree.
  //
  // Requires: s_instance != nullptr
  static HTT_Tree &instanceTree()
    { return s_instance->m_tree; }

  // Return the global tree timer table, or an empty string if the
  // instance was never created.
  static std::string asTableString();
};


// Start/stop a task timer within the `HTT_Singleton`, using a label
// that might vary with each use.
#define HTT_TASK_DYNAMIC_LABEL(label)                             \
  smbase::HTT_LabelIndex SM_PP_CAT(taskLabelIndex_,__LINE__) =    \
    smbase::HTT_Singleton::getIndexForLabel(label);               \
  [[maybe_unused]]                                                \
  smbase::HTT_StartStopLabel SM_PP_CAT(startStopLabel_,__LINE__)( \
    smbase::HTT_Singleton::instanceTree(),                        \
    SM_PP_CAT(taskLabelIndex_,__LINE__)) /* user ; */


// Start/stop using a fixed label.
#define HTT_TASK(label)                \
  static HTT_TASK_DYNAMIC_LABEL(label) /* user ; */


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_HTT_H
