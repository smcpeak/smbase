// tree-print.cc
// Code for tree-print.h.

#include "tree-print.h"                // this module


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
    m_pendingNewline(false),
    m_pendingIndent(0)
{}


TreePrint::PrintState::~PrintState()
{}


void TreePrint::PrintState::flushPendingNewline()
{
  if (m_pendingNewline) {
    m_output << '\n';
  }
  m_pendingNewline = false;
}

void TreePrint::PrintState::emitNewline(int indent)
{
  flushPendingNewline();
  m_pendingNewline = true;
  m_pendingIndent = indent;
}

void TreePrint::PrintState::adjustPendingIndentation(int adj)
{
  if (m_pendingNewline) {
    m_pendingIndent += adj;
    m_availableSpace -= adj;
  }
}

void TreePrint::PrintState::prepareToEmitCharacter()
{
  if (m_pendingNewline) {
    flushPendingNewline();
    printIndent(m_output, m_pendingIndent);
  }
}


// ----------------------------- TPNode --------------------------------
TreePrint::TPNode::TPNode()
  : m_length(0)
{}


TreePrint::TPNode::~TPNode()
{}


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


void TreePrint::TPString::debugPrint(std::ostream &os, int ind) const
{
  printIndent(os, ind);
  os << "TPString: \"" << m_string
     << "\" len=" << m_length << '\n';
}


// --------------------------- TPSequence ------------------------------
TreePrint::TPSequence::TPSequence(int indent, bool consistentBreaks)
  : m_indent(indent),
    m_consistentBreaks(consistentBreaks),
    m_elements(),
    m_lastString(NULL)
{}


TreePrint::TPSequence::~TPSequence()
{}


void TreePrint::TPSequence::addElement(TPNode *element)
{
  m_elements.append(element);
  if (TPString *stringNode = dynamic_cast<TPString*>(element)) {
    m_lastString = stringNode;
  }
  else if (dynamic_cast<TPSequence*>(element)) {
    // Appending a sequence discards the "last string".
    m_lastString = NULL;
  }
}


bool TreePrint::TPSequence::lastElementIsBreak() const
{
  if (m_elements.isNotEmpty()) {
    TPNode const *lastNode = m_elements.lastC();
    if (TPBreak const *breakNode =
          dynamic_cast<TPBreak const *>(lastNode)) {
      if (breakNode->m_breakKind == BK_NEWLINE_ALWAYS) {
        return true;
      }
    }
  }

  return false;
}



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


void TreePrint::TPSequence::debugPrint(std::ostream &os, int ind) const
{
  printIndent(os, ind);
  os << "TPSequence of " << m_elements.count()
     << " elements, length=" << m_length
     << " ind=" << m_indent
     << " consistent=" << (m_consistentBreaks? "true" : "false")
     << ":\n";

  ind += INDENT_SPACES;
  FOREACH_ASTLIST(TPNode, m_elements, iter) {
    iter.data()->debugPrint(os, ind);
  }
}


// ---------------------------- TPBreak --------------------------------
void TreePrint::TPBreak::scan()
{
  // This cannot be (only) done in the constructor because we need to
  // reset the length if the tree is scanned more than once.
  m_length =
    m_breakKind == BK_NEWLINE_OR_SPACE? 1 : 0;
}


void TreePrint::TPBreak::debugPrint(std::ostream &os, int ind) const
{
  printIndent(os, ind);
  os << "TPBreak: " << toString(m_breakKind)
     << " len=" << m_length << '\n';
}


// --------------------------- TreePrint -------------------------------
STATICDEF char const *TreePrint::toString(BreakKind breakKind)
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
  : m_root(0 /*indent*/, false /*consistent*/),
    m_sequenceStack()
{
  m_sequenceStack.push(&m_root);
}


TreePrint::~TreePrint()
{}


void TreePrint::append(TPNode *node)
{
  m_sequenceStack.top()->addElement(node);
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


void TreePrint::begin()
{
  begin(INDENT_SPACES, false /*consistentBreaks*/);
}


void TreePrint::beginConsistent()
{
  begin(INDENT_SPACES, true /*consistentBreaks*/);
}


void TreePrint::begin(int indent, bool consistentBreaks)
{
  TPSequence *seq = new TPSequence(indent, consistentBreaks);
  append(seq);
  m_sequenceStack.push(seq);
}


void TreePrint::end()
{
  // We cannot close 'm_root'.
  xassert(m_sequenceStack.size() > 1);
  m_sequenceStack.pop();
}


bool TreePrint::allSequencesClosed() const
{
  return m_sequenceStack.size() == 1;
}


bool TreePrint::lastElementIsBreak() const
{
  return m_sequenceStack.top()->lastElementIsBreak();
}


bool TreePrint::lastStringIs(char const *str) const
{
  TPString const *stringNode = m_sequenceStack.top()->m_lastString;
  if (stringNode && stringNode->m_string == str) {
    return true;
  }

  return false;
}


// This method uses 'dynamic_cast' to directly inspect the type of the
// children rather than a virtual function because the tight
// communication between the loop and the break nodes, combined with the
// lack thereof for other kinds of nodes, makes the virtual method
// approach a poor fit that tends to obscure the essential logic.
void TreePrint::printSequence(PrintState &printState,
                              TPSequence const *seqNode) const
{
  // Will we force all breaks in this list to be newlines?
  bool forceAllBreaks = false;
  if (seqNode->m_consistentBreaks &&
      seqNode->m_length > printState.m_availableSpace) {
    // We have to break somewhere, so break everywhere.
    forceAllBreaks = true;
  }

  // Establish the indentation level for subsequent lines broken
  // within this sequence.
  int subsequentLineAvailableSpace =
    printState.m_availableSpace - seqNode->m_indent;

  // True if the most recent element was a TPBreak that emitted a
  // newline.
  bool lastWasNewline = false;

  // Print all the elements.
  FOREACH_ASTLIST(TPNode, seqNode->m_elements, iter) {
    TPNode const *node = iter.data();
    lastWasNewline = false;

    // The break nodes are where all the action takes place.
    if (TPBreak const *breakNode =
          dynamic_cast<TPBreak const *>(node)) {
      if (breakNode->m_breakKind == BK_UNINDENT) {
        // Remove some pending indentation.
        printState.adjustPendingIndentation(-INDENT_SPACES);
        continue;
      }

      // If there is not enough space for this break to be a space followed
      // by what comes after, break the line.
      //
      // Or, break the line if the break itself or its parent says to.
      if (breakNode->m_length > printState.m_availableSpace ||
          breakNode->alwaysTaken() ||
          forceAllBreaks)
      {
        // The next line will have available space equal to what was
        // established when this sequence opened.
        printState.m_availableSpace = subsequentLineAvailableSpace ;

        // Emit a newline and indentation to achieve the desired amount
        // of available space.
        lastWasNewline = true;
        printState.emitNewline(
          printState.m_margin - printState.m_availableSpace);
      }

      else {
        // There is enough room for the break character and what follows, so
        // just print the break character.
        if (breakNode->m_breakKind == BK_NEWLINE_OR_SPACE) {
          printState.prepareToEmitCharacter();
          printState.m_output << ' ';
          printState.m_availableSpace--;
        }
      }
    }

    else if (TPString const *stringNode =
               dynamic_cast<TPString const *>(node)) {
      printState.prepareToEmitCharacter();
      printState.m_output << stringNode->m_string;
      printState.m_availableSpace -= stringNode->m_length;
    }

    else if (TPSequence const *innerSeqNode =
               dynamic_cast<TPSequence const *>(node)) {
      printSequence(printState, innerSeqNode);
    }

    else {
      xfailure("unknown TPNode type");
    }
  }

  // If the last element was a break, and this sequence added
  // indentation, remove it from the pending indent before proceeding.
  //
  // This allows sequences to end with breaks without forcing lines that
  // come after to be affected by the indentation within the sequence.
  if (lastWasNewline) {
    printState.adjustPendingIndentation(-seqNode->m_indent);
  }
}


void TreePrint::prettyPrint(std::ostream &os, int margin)
{
  // Compute lengths.
  m_root.scan();

  // Print.
  PrintState printState(os, margin);
  printSequence(printState, &m_root);

  // If there is a pending newline, emit it, but do not print any
  // additional indentation.
  printState.flushPendingNewline();
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
