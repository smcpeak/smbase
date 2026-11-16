// htt.cc
// Code for `htt` module.

// This file is in the public domain.

#include "htt.h"                                 // this module

#include "smbase/fasttime.h"                     // fastTimeInitialize, fastTimeMilliseconds
#include "smbase/ordered-mru-flat-map.h"         // smbase::OrderedMRUFlatMap
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/xassert.h"                      // xassert, xassertPrecondition

#include <string>                                // std::string
#include <string_view>                           // std::string_view


OPEN_NAMESPACE(smbase)


// ----------------------------- HTT_Node ------------------------------
HTT_Node::~HTT_Node()
{}


HTT_Node::HTT_Node(HTT_Node * NULLABLE parent)
  : m_parent(parent),
    m_children(),
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


// ----------------------------- HTT_Tree ------------------------------
HTT_Tree::~HTT_Tree()
{}


HTT_Tree::HTT_Tree()
  : m_labelTable(),
    m_root(nullptr),
    m_stackTop(&m_root)
{
  fastTimeInitialize();
}


HTT_LabelIndex HTT_Tree::getIndexForLabel(std::string_view label)
{
  return m_labelTable.add(label);
}


std::string_view HTT_Tree::getLabelForIndex(HTT_LabelIndex index)
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


std::string HTT_Tree::asString() const
{
  return "TODO";
}


// ------------------------- HTT_StartStopNode -------------------------
HTT_StartStopNode::HTT_StartStopNode(HTT_Node *node)
  : m_node(node),
    m_startMS(fastTimeMilliseconds)
{}


HTT_StartStopNode::~HTT_StartStopNode()
{
  m_node->addElapsedMS(fastTimeMilliseconds - m_startMS);
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
