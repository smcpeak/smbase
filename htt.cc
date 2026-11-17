// htt.cc
// Code for `htt` module.

// This file is in the public domain.

#include "htt.h"                                 // this module

#include "smbase/fasttime.h"                     // fastTimeInitialize, fastTimeMilliseconds
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/ordered-mru-flat-map.h"         // smbase::OrderedMRUFlatMap
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, SELFCHECK
#include "smbase/xassert.h"                      // xassert, xassertPrecondition

#include <string>                                // std::string
#include <string_view>                           // std::string_view
#include <utility>                               // std::move


OPEN_NAMESPACE(smbase)


// ----------------------------- HTT_Node ------------------------------
HTT_Node::~HTT_Node()
{}


void HTT_Node::selfCheck() const
{
  // TODO: Build an iterator for this.
  for (LabelIndexToChildMap::size_type childIndex=0;
       childIndex < m_children.size();
       ++childIndex)
  {
    std::unique_ptr<HTT_Node> const &child =
      m_children.valueAtIndex(childIndex);

    xassert(child != nullptr);
    xassert(child->m_parent == this);

    child->selfCheck();
  }
}


HTT_Node::HTT_Node(HTT_Node * NULLABLE parent)
  : m_parent(parent),
    m_children(),
    m_elapsedMS(0)
{
  SELFCHECK();
}


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


gdv::GDValue HTT_Node::asGDValue(HTT_Tree const &tree) const
{
  gdv::GDValue ret(gdv::GDVK_ORDERED_MAP);

  ret.orderedMapSetValueAtSym("elapsedMS", m_elapsedMS);

  gdv::GDValue children(gdv::GDVK_ORDERED_MAP);

  for (LabelIndexToChildMap::size_type childIndex=0;
       childIndex < m_children.size();
       ++childIndex)
  {
    HTT_LabelIndex labelIndex = m_children.keyAtIndex(childIndex);
    std::string_view label = tree.getLabelForIndex(labelIndex);

    HTT_Node const *child = m_children.valueAtIndex(childIndex).get();
    gdv::GDValue childGDV = child->asGDValue(tree);

    children.orderedMapSetValueAt(
      gdv::GDValue(label), std::move(childGDV));
  }

  ret.orderedMapSetValueAtSym("children", std::move(children));

  return ret;
}


// ----------------------------- HTT_Tree ------------------------------
HTT_Tree::~HTT_Tree()
{}


void HTT_Tree::selfCheck() const
{
  m_labelTable.selfCheck();
  m_root.selfCheck();
  xassert(m_stackTop != nullptr);
}


HTT_Tree::HTT_Tree()
  : m_labelTable(),
    m_root(nullptr),
    m_stackTop(&m_root)
{
  fastTimeInitialize();
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
  m_stackTop = newStackTop;
}


void HTT_Tree::popNode(HTT_Node *oldStackTop)
{
  xassertPrecondition(oldStackTop == m_stackTop);
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


// ------------------------- HTT_StartStopNode -------------------------
HTT_StartStopNode::HTT_StartStopNode(HTT_Node *node)
  : m_node(node),
    m_startMS(fastTimeMilliseconds)
{}


HTT_StartStopNode::~HTT_StartStopNode()
{
  m_node->addElapsedMS(elapsedMSSoFar());
}


unsigned HTT_StartStopNode::elapsedMSSoFar() const
{
  return fastTimeMilliseconds - m_startMS;
}


// ------------------------ HTT_StartStopLabel -------------------------
HTT_StartStopLabel::HTT_StartStopLabel(
  HTT_Tree &tree,
  HTT_LabelIndex labelIndex)
  : HTT_StartStopNode(tree.getTOSChildNode(labelIndex)),
    m_tree(tree)
{
  m_tree.pushNode(getNode());
}


HTT_StartStopLabel::~HTT_StartStopLabel()
{
  m_tree.popNode(getNode());
}


CLOSE_NAMESPACE(smbase)


// EOF
