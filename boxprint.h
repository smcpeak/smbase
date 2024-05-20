// boxprint.h
// BoxPrint functions as an output stream (sort of like <code>cout</code>)
// with operations to indicate structure within the emitted text, so that
// it can break lines intelligently.  It's used as part of a source-code
// pretty-printer.

// It is partly based the box model described at
// http://caml.inria.fr/FAQ/format-eng.html

#ifndef BOXPRINT_H
#define BOXPRINT_H

#include "str.h"          // stringBuilder
#include "astlist.h"      // ASTList
#include "array.h"        // ObjArrayStack


// fwd
class BoxPrint;


// manages the process of rendering a boxprint tree to a string
class BPRender {
public:
  // output string
  stringBuilder sb;

  // right margin column; defaults to 72
  int margin;

  // column for next output; equal to the number of characters
  // after the last newline in 'sb'
  int curCol;

  // text to begin every line with; not counted towards column
  // counts; defaults to ""
  OldSmbaseString lineStartText;

public:
  BPRender();
  ~BPRender();

  // chars in the current line
  int getCurCol() const { return curCol; }

  // chars remaining on current line before the margin; might
  // be negative if the input didn't have enough breaks
  int remainder() const { return margin - getCurCol(); }

  // add some text (that doesn't include newlines) to the output
  void add(rostring text);

  // add a newline, plus indentation to get to column 'ind'
  void breakLine(int ind);

  // take the string out of the rendering engine, replacing it
  // with the empty string
  OldSmbaseString takeString() {
    OldSmbaseString ret(sb);
    reset();
    return ret;
  }

  // just clear the buffer to its original state; must do this
  // manually after changing 'lineStartText'
  void reset();

  // take the tree out of a boxprint builder, convert it to a string,
  // and delete the tree
  OldSmbaseString takeAndRender(BoxPrint &bld);
};


// interface for elements in a boxprint tree
class BPElement {
public:
  // if no breaks are taken, compute the # of columns;
  // return with 'forcedBreak' true if we stopped because of
  // a forced break
  virtual int oneLineWidthEx(bool &forcedBreak)=0;

  // as above, but without the forcedBreak info
  int oneLineWidth();

  // render this element as a string with newlines, etc.
  virtual void render(BPRender &mgr)=0;

  // true if this element is a BPBreak and is enabled; returns false
  // by default
  virtual bool isBreak() const;

  // true if is BPBreak and BT_FORCED
  virtual bool isForcedBreak() const;

  // print the boxprint tree; for debugging code that produces them;
  // these methods do not emit leading or trailing whitespace
  virtual void debugPrint(ostream &os, int ind) const =0;

  // deallocate the element
  virtual ~BPElement();
};


// leaf in the tree: text to print
class BPText : public BPElement {
public:
  OldSmbaseString text;

public:
  BPText(rostring t);
  ~BPText();

  // BPElement funcs
  virtual int oneLineWidthEx(bool &forcedBreak) override;
  virtual void render(BPRender &mgr) override;
  virtual void debugPrint(ostream &os, int ind) const override;
};


enum BreakType {
  // Equivalent to a space.
  BT_DISABLED = 0,

  // Either a space or a newline depending on the context.
  BT_ENABLED = 1,

  // Always a newline.
  BT_FORCED = 2,

  // Newline if the cursor is not at the start of the line.
  BT_LINE_START = 3,
};

// leaf in the tree: a "break", which might end up being a
// space or a newline+indentation
class BPBreak : public BPElement {
public:
  // Type of break.
  BreakType m_breakType;

  // Nominally, when a break is taken, the indentation used is such
  // that the next char in the box is directly below the first char
  // in the box.  When this break is passed, however, it can add to
  // that nominal indent of 0; these adjustments accumulate as the
  // box is rendered.
  //
  // This value is the number of spaces to indent.  Usually, it is a
  // multiple of BoxPrint::levelIndent.
  int m_indent;

public:
  BPBreak(BreakType breakType, int indent);
  ~BPBreak();

  // BPElement funcs
  virtual int oneLineWidthEx(bool &forcedBreak) override;
  virtual void render(BPRender &mgr) override;
  virtual bool isBreak() const override;
  virtual bool isForcedBreak() const override;
  virtual void debugPrint(ostream &os, int ind) const override;
};


// kinds of boxes
enum BPKind {
  // enabled breaks are always taken
  BP_vertical,

  // enabled breaks are individually taken or not taken depending
  // on how much room is available; "hov"
  BP_sequence,

  // either all enabled breaks are taken, or none are taken; "h/v"
  BP_correlated,

  // # of kinds, also used to signal the end of a box in some cases
  NUM_BPKINDS
};

// internal node in the tree: a list of subtrees, some of which
// may be breaks
class BPBox : public BPElement {
public:
  // subtrees
  ASTList<BPElement> elts;

  // break strategy for this box
  BPKind kind;

public:
  BPBox(BPKind k);
  ~BPBox();

  // BPElement funcs
  virtual int oneLineWidthEx(bool &forcedBreak) override;
  virtual void render(BPRender &mgr) override;
  virtual void debugPrint(ostream &os, int ind) const override;
};


// assists in the process of building a box tree by providing
// a number of syntactic shortcuts
class BoxPrint {
public:      // types
  // additional command besides BPKind
  enum Cmd {
    sp,          // insert disabled break
    br,          // insert enabled break
    fbr,         // insert forced break
    lineStart,   // insert lineStart break
    ind,         // ibr(levelIndent)
    und,         // adjustIndent(-1); no break
  };

  // insert enabled break with indentation
  struct IBreak {
    int indent;
    IBreak(int i) : indent(i) {}
    // use default copy ctor
  };

  // operator sequence
  struct Op {
    char const *text;
    Op(char const *t) : text(t) {}
    // default copy ctor
  };

private:     // data
  // stack of open boxes; always one open vert box at the top
  ObjArrayStack<BPBox> boxStack;

public:      // data
  // convenient names for the box kinds
  static BPKind const vert;       // = BP_vertical
  static BPKind const seq;        // = BP_sequence
  static BPKind const hv;         // = BP_correlated ("h/v")
  static BPKind const end;        // = NUM_BPKINDS

  // indentation amount for the ind/und commands; defaults to 2
  int levelIndent;

private:     // funcs
  // innermost box being built
  BPBox *box() { return boxStack.top(); }

public:      // funcs
  BoxPrint();
  ~BoxPrint();

  // append another element to the current innermost box
  void append(BPElement *elt);

  // add BPText nodes to current box
  BoxPrint& operator<< (rostring s);
  BoxPrint& operator<< (char const *s);
  BoxPrint& operator<< (int i);
  BoxPrint& operator<< (char c);

  // open/close boxes
  BoxPrint& operator<< (BPKind k);

  // insert breaks
  BoxPrint& operator<< (Cmd c);

  // insert break with indentation
  static IBreak ibr(int i) { return IBreak(i); }
  BoxPrint& operator<< (IBreak b);

  // op(text) is equivalent to sp << text << br
  static Op op(char const *text) { return Op(text); }
  BoxPrint &operator << (Op o);

  // Adjust indentation for remainder of this box.  'steps' is
  // multiplied by 'levelIndent' to get the number of spaces.  This
  // can only be right after a break.
  void adjustIndent(int steps);

  // take the accumulated box tree out; all opened boxes must have
  // been closed; the builder is left in a state where it can be used
  // to build a new tree if desired, or it can be simply destroyed
  BPBox* /*owner*/ takeTree();

  // print the current stack of trees
  void debugPrint(ostream &os) const;
  void debugPrintCout() const;      // for gdb
};


#endif // BOXPRINT_H
