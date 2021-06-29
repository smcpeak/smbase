// tree-print.cc
// Code for tree-print.h.

#include "tree-print.h"                // this module

#include "sm-macros.h"                 // RESTORER


enum {
  // Number of spaces per indent level.
  INDENT_SPACES = 2,
};


static std::ostream &printIndent(std::ostream &os, int ind)
{
  while (ind-- > 0) {
    os << ' ';
  }
  return os;
}


// --------------------------- PrintState ------------------------------
TreePrint::PrintState::PrintState(std::ostream &output, int margin)
  : m_output(output),
    m_margin(margin),
    m_availableSpace(margin),
    m_availableSpaceStack()
{}


TreePrint::PrintState::~PrintState()
{}


// ----------------------------- TPNode --------------------------------
TreePrint::TPNode::TPNode()
  : m_length(0)
{}


TreePrint::TPNode::~TPNode()
{}


// --------------------------- TPSequence ------------------------------
TreePrint::TPSequence::TPSequence(int indent)
  : m_indent(indent),
    m_elements()
{}


TreePrint::TPSequence::~TPSequence()
{}


// The 'scan' algorithm described in section 3 of the Oppen paper is
// remarkably difficult to understand.  My implementation is based on
// the English text that describes what the "associated integer" for
// each token is.  In my implementation, that integer is 'm_length'.
void TreePrint::TPSequence::scan()
{
  // Reset the length for this node so we can safely re-scan more than
  // once.
  m_length = 0;

  // Most recently seen break in this sequence.
  TPBreak *lastBreak = NULL;

  // Length of the most recently seen break plus all non-break nodes
  // that followed it.
  int lengthFromLastBreak = 0;

  // Scan the children.
  FOREACH_ASTLIST_NC(TPNode, m_elements, iter) {
    TPNode *node = iter.data();

    // Ask the child to compute its length recursively.
    node->scan();
    int len = node->m_length;

    // Accumulate child lengths in this sequence length.
    m_length += len;

    // When we see a break node, set its length to be its own length
    // (currently stored in 'len') plus the lengths of all following
    // non-break nodes.
    if (TPBreak *atBreak = dynamic_cast<TPBreak*>(node)) {
      if (lastBreak) {
        // Update the previous break.
        lastBreak->m_length = lengthFromLastBreak;
      }

      // Initialize 'lengthFromLastBreak'.
      lengthFromLastBreak = len;

      // Remember this break so we can update its length later.
      lastBreak = atBreak;
    }
    else {
      // Accumulate the lengths of non-break nodes.
      lengthFromLastBreak += len;
    }
  }

  if (lastBreak) {
    // Update the last break.
    lastBreak->m_length = lengthFromLastBreak;
  }
}


void TreePrint::TPSequence::print(PrintState &printState) const
{
  // Establish the indentation level for subsequent lines broken
  // within this sequence.
  printState.m_availableSpaceStack.push(
    printState.m_availableSpace - m_indent);

  FOREACH_ASTLIST(TPNode, m_elements, iter) {
    iter.data()->print(printState);
  }

  printState.m_availableSpaceStack.pop();
}


void TreePrint::TPSequence::debugPrint(std::ostream &os, int ind) const
{
  printIndent(os, ind);
  os << "TPSequence of " << m_elements.count()
     << " elements, length=" << m_length
     << " ind=" << m_indent
     << ":\n";

  ind += 2;
  FOREACH_ASTLIST(TPNode, m_elements, iter) {
    iter.data()->debugPrint(os, ind);
  }
}


// ---------------------------- TPString -------------------------------
TreePrint::TPString::TPString(string const &s)
  : m_string(s)
{
  m_length = m_string.length();
}


TreePrint::TPString::~TPString()
{}


void TreePrint::TPString::scan()
{
  // 'm_length' is set upon construction, and the string cannot be
  // changed afterward, so we don't need to do anything here.
}


void TreePrint::TPString::print(PrintState &printState) const
{
  printState.m_output << m_string;
  printState.m_availableSpace -= m_length;
}


void TreePrint::TPString::debugPrint(std::ostream &os, int ind) const
{
  printIndent(os, ind);
  os << "TPString: \"" << m_string
     << "\" len=" << m_length << '\n';
}


// ---------------------------- TPBreak --------------------------------
void TreePrint::TPBreak::scan()
{
  // This cannot be (only) done in the constructor because we need to
  // reset the length if the tree is scanned more than once.
  m_length =
    m_breakKind == BK_NEWLINE_OR_SPACE? 1 : 0;
}


void TreePrint::TPBreak::print(PrintState &printState) const
{
  // If there is not enough space for this break to be a space followed
  // by what comes after, break the line.  (Or break it if the break is
  // unconditional.)
  if (m_length > printState.m_availableSpace ||
      m_breakKind == BK_NEWLINE_ALWAYS) {
    // The next line will have available space equal to what was
    // established when the innermost block opened.
    printState.m_availableSpace = printState.m_availableSpaceStack.top();

    // Add a newline and indent such that the desired available space is
    // in fact what is available.
    printState.m_output << '\n';
    printIndent(printState.m_output,
      printState.m_margin - printState.m_availableSpace);
  }

  else {
    // There is enough room for the break character and what follows, so
    // just print the break character.
    if (m_breakKind == BK_NEWLINE_OR_SPACE) {
      printState.m_output << ' ';
      printState.m_availableSpace--;
    }
  }
}


void TreePrint::TPBreak::debugPrint(std::ostream &os, int ind) const
{
  printIndent(os, ind);
  os << "TPBreak: " << toString(m_breakKind)
     << " len=" << m_length << '\n';
}


// --------------------------- TreePrint -------------------------------
/*static*/ char const *TreePrint::toString(BreakKind breakKind)
{
  static char const * const names[] = {
    "BK_NEWLINE_ALWAYS",
    "BK_NEWLINE_OR_SPACE",
    "BK_NEWLINE_OR_NOTHING",
    "BK_UNINDENT",
  };
  ASSERT_TABLESIZE(names, NUM_BREAK_KINDS);

  if ((unsigned)breakKind < NUM_BREAK_KINDS) {
    return names[breakKind];
  }
  else {
    return "bad BreakKind";
  }
}


TreePrint::TreePrint()
  : m_root(0 /*indent*/),
    m_sequenceStack()
{
  m_sequenceStack.push(&m_root);
}


TreePrint::~TreePrint()
{}


void TreePrint::append(TPNode *node)
{
  m_sequenceStack.top()->m_elements.append(node);
}


TreePrint& TreePrint::operator<< (char const *str)
{
  return operator<< (string(str));
}


TreePrint& TreePrint::operator<< (int i)
{
  return operator<< (stringb(i));
}


TreePrint& TreePrint::operator<< (char c)
{
  return operator<< (stringb(c));
}


TreePrint& TreePrint::operator<< (string const &str)
{
  append(new TPString(str));
  return *this;
}


TreePrint& TreePrint::operator<< (BreakKind breakKind)
{
  append(new TPBreak(breakKind));
  return *this;
}


TreePrint& TreePrint::operator<< (SequenceCommand sequenceCommand)
{
  int indent = 0;

  switch (sequenceCommand) {
    default:
      xfailure("bad sequence command");

    case SC_BEGIN_INDENT:
      indent = INDENT_SPACES;
      // fallthrough

    case SC_BEGIN_NO_INDENT: {
      TPSequence *seq = new TPSequence(indent);
      append(seq);
      m_sequenceStack.push(seq);
      break;
    }

    case SC_END: {
      // We cannot close 'm_root'.
      xassert(m_sequenceStack.size() > 1);
      m_sequenceStack.pop();
      break;
    }
  }

  return *this;
}


bool TreePrint::allSequencesClosed() const
{
  return m_sequenceStack.size() == 1;
}


void TreePrint::print(std::ostream &os, int margin)
{
  // Compute lengths.
  m_root.scan();

  // Print.
  PrintState printState(os, margin);
  m_root.print(printState);
}


void TreePrint::clear()
{
  m_root.m_elements.deleteAll();
  while (m_sequenceStack.size() > 1) {
    m_sequenceStack.pop();
  }
  xassert(allSequencesClosed());
  xassert(m_sequenceStack.top() == &m_root);
}


void TreePrint::debugPrint(std::ostream &os) const
{
  m_root.debugPrint(os, 0 /*ind*/);
}


void TreePrint::debugPrintCout() const
{
  debugPrint(std::cout);
}


// EOF
