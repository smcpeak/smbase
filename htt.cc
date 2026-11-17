// htt.cc
// Code for `htt` module.

// This file is in the public domain.

#include "htt.h"                                 // this module

#include "smbase/fasttime.h"                     // smbase::{fastTimeInitialize, fastTimeMilliseconds, fastTimeIsImplemented}
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/ordered-mru-flat-map.h"         // smbase::OrderedMRUFlatMap
#include "smbase/set-util.h"                     // smbase::{setContains, setInsertUnique}
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, SELFCHECK, IMEMBFP
#include "smbase/stringb.h"                      // stringb
#include "smbase/string-util.h"                  // repeatString
#include "smbase/text-table.h"                   // smbase::TextTable
#include "smbase/xassert.h"                      // xassert, xassertPrecondition

#include <string>                                // std::string
#include <string_view>                           // std::string_view
#include <utility>                               // std::move


OPEN_NAMESPACE(smbase)


// ----------------------------- HTT_Node ------------------------------
HTT_Node::~HTT_Node()
{}


void HTT_Node::selfCheck(std::set<HTT_Node const *> const &stackNodes) const
{
  // The set of running nodes should be the same as the set of nodes on
  // the stack.
  xassert(m_running == setContains(stackNodes, this));

  for (auto [_, child] : m_children) {
    xassert(child != nullptr);
    xassert(child->m_parent == this);

    child->selfCheck(stackNodes);
  }
}


HTT_Node::HTT_Node(HTT_Node * NULLABLE parent)
  : IMEMBFP(parent),
    m_children(),
    m_running(false),
    m_startMS(0),
    m_elapsedMS(0)
{}


HTT_Node *HTT_Node::getChild(HTT_LabelIndex labelIndex)
{
  // Lookup `labelIndex`.
  std::unique_ptr<HTT_Node> &nodePtr =
    m_children.valueRefAtKey(labelIndex);

  if (!nodePtr) {
    // It wasn't previously present, so make the node now.
    nodePtr.reset(new HTT_Node(this));
  }

  xassert(nodePtr->m_parent == this);
  return nodePtr.get();
}


void HTT_Node::start()
{
  xassertdb(running() == false);

  m_startMS = fastTimeMilliseconds;
  m_running = true;
}


void HTT_Node::stop()
{
  xassertdb(running() == true);

  m_elapsedMS += (fastTimeMilliseconds - m_startMS);
  m_running = false;
}


unsigned HTT_Node::elapsedPlusRunningMS(unsigned currentMS) const
{
  unsigned ret = m_elapsedMS;

  if (running()) {
    ret += (currentMS - m_startMS);
  }

  return ret;
}


gdv::GDValue HTT_Node::asGDValue(HTT_Tree const &tree) const
{
  using gdv::toGDValue;

  gdv::GDValue m(gdv::GDVK_ORDERED_MAP);

  GDV_WRITE_MEMBER_SYM(m_running);
  GDV_WRITE_MEMBER_SYM(m_startMS);
  GDV_WRITE_MEMBER_SYM(m_elapsedMS);

  gdv::GDValue children(gdv::GDVK_ORDERED_MAP);

  for (auto [labelIndex, child] : m_children) {
    std::string_view label = tree.getLabelForIndex(labelIndex);

    gdv::GDValue childGDV = child->asGDValue(tree);

    children.orderedMapSetValueAt(
      gdv::GDValue(label), std::move(childGDV));
  }

  m.orderedMapSetValueAtSym("children", std::move(children));

  return m;
}


void HTT_Node::renderTableRows(
  TextTable &tt,
  HTT_Tree const &tree,
  std::string_view label,
  int indent,
  unsigned currentMS) const
{
  tt.m_cells.push_back(std::vector<TextTableCell>{
    TextTableCell(stringb(repeatString("  ", indent) << label)),
    TextTableCell(elapsedPlusRunningMS(currentMS)),
  });

  if (running()) {
    tt.m_cells.back().push_back(TextTableCell("r"));
  }

  for (auto [labelIndex, child] : m_children) {
    std::string_view label = tree.getLabelForIndex(labelIndex);

    child->renderTableRows(tt, tree, label, indent+1, currentMS);
  }
}


// ----------------------------- HTT_Tree ------------------------------
HTT_Tree::~HTT_Tree()
{
  // Although I don't enforce this, normally when a timer node is
  // destroyed, it should not be running.
  m_root.stop();
}


void HTT_Tree::selfCheck() const
{
  m_labelTable.selfCheck();

  // Get the set of nodes on the stack.
  std::set<HTT_Node const *> stackNodes;
  for (HTT_Node const *n = m_stackTop; n != nullptr; n = n->getParent()) {
    // Using the "Unique" variant here ensures we can't get stuck in an
    // infinite loop.
    setInsertUnique(stackNodes, n);
  }

  // Recursively check the tree, taking into account what is on the
  // stack.
  m_root.selfCheck(stackNodes);

  xassert(m_stackTop != nullptr);
}


HTT_Tree::HTT_Tree()
  : m_labelTable(),
    m_root(nullptr),
    m_stackTop(&m_root)
{
  fastTimeInitialize();

  // The root node will never accumulate anything in its `m_elapsedMS`
  // (except right before destruction), but the fact that it is running
  // will be taken into account when the table is rendered.
  m_root.start();

  SELFCHECK();
}


HTT_LabelIndex HTT_Tree::getIndexForLabel(std::string_view label)
{
  return m_labelTable.add(label);
}


std::string_view HTT_Tree::getLabelForIndex(HTT_LabelIndex index) const
{
  return m_labelTable.get(index);
}


HTT_Node *HTT_Tree::getTOSChildNode(HTT_LabelIndex labelIndex)
{
  return m_stackTop->getChild(labelIndex);
}


void HTT_Tree::pushNode(HTT_Node *newStackTop)
{
  xassertPrecondition(newStackTop->getParent() == m_stackTop);
  xassertPrecondition(newStackTop->running() == false);

  m_stackTop = newStackTop;
  m_stackTop->start();
}


void HTT_Tree::popNode(HTT_Node *oldStackTop)
{
  xassertPrecondition(oldStackTop == m_stackTop);
  xassertPrecondition(oldStackTop->running() == true);

  m_stackTop->stop();
  m_stackTop = oldStackTop->getParent();

  xassert(m_stackTop != nullptr);
}


HTT_Tree::operator gdv::GDValue() const
{
  return m_root.asGDValue(*this);
}


std::string HTT_Tree::asString() const
{
  gdv::GDValue v(*this);
  return v.asIndentedString();
}


std::string HTT_Tree::asTableString() const
{
  TextTable tt;
  tt.m_columnTitles = std::vector<std::string>{
    "timer",
    "elapsed_ms",
    "r",
  };

  // Sample the clock once up front to avoid possibly producing a table
  // with inconsistent timing data.
  unsigned const currentMS = fastTimeMilliseconds;

  m_root.renderTableRows(tt, *this, "root", 0 /*indent*/, currentMS);

  std::string table = tt.render();

  if (!fastTimeIsImplemented()) {
    table += "NOTE: `fasttime` is not implemented on this platform, "
             "so all times are zero.\n";
  }

  return table;
}


// ------------------------- HTT_StartStopNode -------------------------
HTT_StartStopNode::HTT_StartStopNode(HTT_Tree &tree, HTT_Node *node)
  : IMEMBFP(tree),
    IMEMBFP(node)
{
  m_tree.pushNode(m_node);
}


HTT_StartStopNode::~HTT_StartStopNode()
{
  m_tree.popNode(m_node);
}


// ------------------------ HTT_StartStopLabel -------------------------
HTT_StartStopLabel::HTT_StartStopLabel(
  HTT_Tree &tree,
  HTT_LabelIndex labelIndex)
  : HTT_StartStopNode(tree, tree.getTOSChildNode(labelIndex))
{}


HTT_StartStopLabel::~HTT_StartStopLabel()
{}


// --------------------------- HTT_Singleton ---------------------------
HTT_Singleton * NULLABLE HTT_Singleton::s_instance = nullptr;


HTT_Singleton::HTT_Singleton()
  : m_tree()
{}


/*static*/ void HTT_Singleton::ensureCreated()
{
  if (s_instance == nullptr) {
    // This object is never deallocated.
    s_instance = new HTT_Singleton();
  }
}


/*static*/ HTT_LabelIndex HTT_Singleton::getIndexForLabel(
  std::string_view label)
{
  ensureCreated();

  return instanceTree().getIndexForLabel(label);
}


/*static*/ std::string HTT_Singleton::asTableString()
{
  if (s_instance == nullptr) {
    return "";
  }

  else {
    return instanceTree().asTableString();
  }
}


CLOSE_NAMESPACE(smbase)


// EOF
