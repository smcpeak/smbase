// boxprint.cc
// code for boxprint.h

#include "boxprint.h"                  // this module

#include "string-util.h"               // doubleQuote
#include "stringb.h"                   // stringb
#include "xassert.h"                   // xassert

#include <string.h>                    // strlen


// ----------------------- BPRender ----------------------
BPRender::BPRender()
  : sb(),         // initially empty
    margin(72),
    curCol(0),
    lineStartText("")
{}

BPRender::~BPRender()
{}


void BPRender::reset()
{
  sb.clear();
  sb << lineStartText;
}


void BPRender::add(rostring text)
{
  int len = strlen(text);
  sb << text;
  curCol += len;
}

void BPRender::breakLine(int ind)
{
  sb << "\n" << lineStartText;

  for (int i=0; i < ind; i++) {
    sb << ' ';
  }

  curCol = ind;
}


string BPRender::takeAndRender(BoxPrint &bld)
{
  BPBox* /*owner*/ tree = bld.takeTree();
  tree->render(*this);
  string ret(sb);
  sb.clear();
  delete tree;
  return ret;
}


// ----------------------- BPElement ---------------------
int BPElement::oneLineWidth()
{
  bool dummy;
  return oneLineWidthEx(dummy);
}

bool BPElement::isBreak() const
{
  return false;
}

bool BPElement::isForcedBreak() const
{
  return false;
}

BPElement::~BPElement()
{}


// ------------------------- BPText ----------------------
BPText::BPText(rostring t)
  : text(t)
{}

BPText::~BPText()
{}


int BPText::oneLineWidthEx(bool &forced)
{
  forced = false;
  return text.length();
}

void BPText::render(BPRender &mgr)
{
  mgr.add(text);
}


void BPText::debugPrint(ostream &os, int /*ind*/) const
{
  os << "text(" << doubleQuote(text) << ")";
}


// ------------------------ BPBreak ---------------------
BPBreak::BPBreak(BreakType breakType, int indent)
  : m_breakType(breakType),
    m_indent(indent)
{}

BPBreak::~BPBreak()
{}

int BPBreak::oneLineWidthEx(bool &forced)
{
  if (m_breakType >= BT_FORCED) {
    forced = true;
    return 0;
  }
  else {
    forced = false;
    return 1;
  }
}

void BPBreak::render(BPRender &mgr)
{
  // if we're being asked to render, then this break must not be taken
  if (m_breakType != BT_LINE_START) {
    mgr.add(" ");
  }
}

bool BPBreak::isBreak() const
{
  return m_breakType != BT_DISABLED;
}

bool BPBreak::isForcedBreak() const
{
  return m_breakType == BT_FORCED;
}

void BPBreak::debugPrint(ostream &os, int /*ind*/) const
{
  os << "break(en=" << (int)m_breakType << ", ind=" << m_indent << ")";
}


// ------------------------- BPBox ------------------------
BPBox::BPBox(BPKind k)
  : elts(),      // initially empty
    kind(k)
{
  xassert((unsigned)k < NUM_BPKINDS);
}

BPBox::~BPBox()
{}


int BPBox::oneLineWidthEx(bool &forced)
{
  forced = false;
  int sum = 0;
  FOREACH_ASTLIST_NC(BPElement, elts, iter) {
    sum += iter.data()->oneLineWidthEx(forced);
    if (forced) {
      break;
    }
  }
  return sum;
}


void takeBreak(BPRender &mgr, int &startCol, BPBreak *brk)
{
  startCol += brk->m_indent;

  if (brk->m_breakType == BT_LINE_START &&
      mgr.curCol == startCol) {
    // do not add a line
  }
  else {
    // add the newline
    mgr.breakLine(startCol);
  }
}


// this function is the heart of the rendering engine
void BPBox::render(BPRender &mgr)
{
  int startCol = mgr.getCurCol();

  bool fbreak = false;
  if (kind == BP_vertical ||
      (kind == BP_correlated && (oneLineWidthEx(/*OUT*/fbreak) > mgr.remainder() ||
                                 fbreak))) {
    // take all of the breaks
    FOREACH_ASTLIST_NC(BPElement, elts, iter) {
      BPElement *elt = iter.data();
      if (elt->isBreak()) {
        takeBreak(mgr, startCol, static_cast<BPBreak*>(elt));
      }
      else {
        elt->render(mgr);
      }
    }
    return;
  }

  if (kind == BP_correlated) {
    // if we got here, we're taking none of the breaks
    FOREACH_ASTLIST_NC(BPElement, elts, iter) {
      BPElement *elt = iter.data();
      elt->render(mgr);
    }
    return;
  }

  xassert(kind == BP_sequence);

  // this cursor points to the next element that has not been rendered
  ASTListIterNC<BPElement> cursor(elts);

  // when not NULL, the cursor has just passed a break, but we haven't
  // actually decided whether to take it or not
  BPBreak *pendingBreak = NULL;

  while (!cursor.isDone()) {
    // is there room for the elements up to the first break?
    int segmentWidth = pendingBreak? 1 : 0;
    ASTListIterNC<BPElement> lookahead(cursor);
    while (!lookahead.isDone() && !lookahead.data()->isBreak()) {
      segmentWidth += lookahead.data()->oneLineWidth();
      lookahead.adv();
    }

    if (pendingBreak && segmentWidth > mgr.remainder()) {
      // take the pending break
      takeBreak(mgr, startCol, pendingBreak);
      pendingBreak = NULL;
    }

    // the segment will be put here without a preceding break
    else if (pendingBreak) {
      startCol += pendingBreak->m_indent;
      pendingBreak->render(mgr);
      pendingBreak = NULL;
    }

    xassert(pendingBreak == NULL);

    // render the segment
    while (!cursor.isDone() && !cursor.data()->isBreak()) {
      cursor.data()->render(mgr);
      cursor.adv();
    }

    if (!cursor.isDone()) {
      // we stopped on a break
      pendingBreak = static_cast<BPBreak*>(cursor.data());

      if (pendingBreak->isForcedBreak()) {
        // take the forced break
        takeBreak(mgr, startCol, pendingBreak);
        pendingBreak = NULL;
      }

      cursor.adv();
    }
  }

  if (pendingBreak) {
    // ended with a break.. strange, but harmless I suppose
    pendingBreak->render(mgr);

    if (pendingBreak->isForcedBreak()) {
      takeBreak(mgr, startCol, pendingBreak);
    }
  }
}


void BPBox::debugPrint(ostream &os, int ind) const
{
  static char const * const map[] = {
    "vert",
    "seq",
    "corr"
  };

  os << "box(kind=" << map[kind] << ") {\n";
  ind += 2;

  FOREACH_ASTLIST(BPElement, elts, iter) {
    for (int i=0; i<ind; i++) {
      os << " ";
    }

    iter.data()->debugPrint(os, ind);
    os << "\n";
  }

  ind -= 2;
  for (int i=0; i<ind; i++) {
    os << " ";
  }
  os << "}";
}


// ------------------------ BoxPrint ----------------------
BPKind const BoxPrint::vert = BP_vertical;
BPKind const BoxPrint::seq  = BP_sequence;
BPKind const BoxPrint::hv   = BP_correlated;
BPKind const BoxPrint::end  = NUM_BPKINDS;


BoxPrint::BoxPrint()
  : boxStack(),
    levelIndent(2)
{
  // initial vert box
  boxStack.push(new BPBox(BP_vertical));
}

BoxPrint::~BoxPrint()
{}


void BoxPrint::append(BPElement *elt)
{
  box()->elts.append(elt);
}


BoxPrint& BoxPrint::operator<< (rostring s)
{
  append(new BPText(s));
  return *this;
}

BoxPrint& BoxPrint::operator<< (char const *s)
{
  append(new BPText(s));
  return *this;
}

BoxPrint& BoxPrint::operator<< (int i)
{
  return operator<< (stringb(i));
}

BoxPrint& BoxPrint::operator<< (char c)
{
  return operator<< (stringb(c));
}


BoxPrint& BoxPrint::operator<< (BPKind k)
{
  if (k == NUM_BPKINDS) {
    // close current box
    append(boxStack.pop());
  }
  else {
    // open new box
    boxStack.push(new BPBox(k));
  }
  return *this;
}


BoxPrint& BoxPrint::operator<< (Cmd c)
{
  switch (c) {
    default: xfailure("bad cmd");
    case sp:        append(new BPBreak(BT_DISABLED, 0 /*indent*/)); break;
    case br:        append(new BPBreak(BT_ENABLED, 0 /*indent*/)); break;
    case fbr:       append(new BPBreak(BT_FORCED, 0 /*indent*/)); break;
    case lineStart: append(new BPBreak(BT_LINE_START, 0 /*indent*/)); break;
    case ind:       append(new BPBreak(BT_ENABLED, levelIndent)); break;
    case und:       adjustIndent(-1); break;
  }
  return *this;
}


BoxPrint& BoxPrint::operator<< (IBreak b)
{
  append(new BPBreak(BT_ENABLED, b.indent /*indent*/));
  return *this;
}


BoxPrint& BoxPrint::operator<< (Op o)
{
  return *this << sp << o.text << br;
}


void BoxPrint::adjustIndent(int steps)
{
  if (box()->elts.isEmpty()) {
    *this << "[ERROR:adjustIndent called on empty box]";
  }
  else {
    BPElement *last = box()->elts.last();
    BPBreak *lastBreak = dynamic_cast<BPBreak*>(last);
    if (!lastBreak) {
      *this << "[ERROR:adjustIndent called when prev element not a break]";
    }
    else {
      lastBreak->m_indent += steps * levelIndent;
    }
  }
}


BPBox* /*owner*/ BoxPrint::takeTree()
{
  // all boxes must be closed
  xassert(boxStack.length() == 1);

  BPBox *ret = boxStack.pop();

  // initialize the box stack again, in case the user wants
  // to build another tree
  boxStack.push(new BPBox(BP_vertical));

  return ret;
}


void BoxPrint::debugPrint(ostream &os) const
{
  for (int i=0; i < boxStack.length(); i++) {
    os << "----- frame -----\n";
    boxStack[i]->debugPrint(os, 0 /*ind*/);
    os << "\n";
  }
}

void BoxPrint::debugPrintCout() const
{
  debugPrint(cout);
}


// EOF
