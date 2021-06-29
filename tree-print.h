// tree-print.h
// TreePrint class.

// This is yet another attempt at pretty-printing that is adequate for
// printing C++ source in a readable manner.  BoxPrint is not working
// the way I want, and it's not obvious how to fix it, so I'm trying a
// somewhat different approach.
//
// The algorithm in this module is modeled on:
//
//   Prettyprinting
//   Derek C. Oppen
//   ACM TOPLAS, Vol. 2, No. 4, October 1980, pp. 465-483.
//   https://www.cs.tufts.edu/~nr/cs257/archive/derek-oppen/prettyprinting.pdf

#ifndef SMBASE_TREE_PRINT_H
#define SMBASE_TREE_PRINT_H

// smbase
#include "astlist.h"                   // ASTList
#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "str.h"                       // string

// libc++
#include <iostream>                    // std::ostream
#include <stack>                       // std::stack


// This class holds a tree to print while it is being built.  It then
// prints that tree to an ostream in the 'print()' method.
class TreePrint {
  NO_OBJECT_COPIES(TreePrint);

private:     // types
  // State maintained during printing.
  class PrintState {
  public:      // data
    // Output stream.
    std::ostream &m_output;

    // Desired max width, but we can go over if necessary.
    int m_margin;

    // Number of characters remaining in the current line before we
    // overrun the margin.
    int m_availableSpace;

    // Stack of previous available space counts.  An element is pushed
    // when we go into a sequence and popped when we return.
    std::stack<int> m_availableSpaceStack;

  public:      // methods
    PrintState(std::ostream &output, int margin);
    ~PrintState();
  };

  // Abstract base class for nodes in the tree.
  class TPNode {
  public:      // data
    // Number of characters in this node.
    //
    // For a break, this is the length of the break if *not* taken, plus
    // the length of what follows in the same sequence, up to the next
    // break.
    //
    // This is calculated by 'scan()'.
    int m_length;

  public:      // methods
    TPNode();
    virtual ~TPNode();

    // Calculate 'm_length'.
    virtual void scan() = 0;

    // Print this node.
    virtual void print(PrintState &printState) const = 0;

    // Print the tree structure itself.
    virtual void debugPrint(std::ostream &os, int ind) const = 0;
  };

  // Interior node containing a sequence of nodes.
  class TPSequence : public TPNode {
  public:      // data
    // How many spaces to indent second-and-later lines within this
    // sequence.
    int m_indent;

    // Subtrees.
    ASTList<TPNode> m_elements;

  public:
    TPSequence(int indent);
    ~TPSequence();

    virtual void scan() override;
    virtual void print(PrintState &printState) const override;
    virtual void debugPrint(std::ostream &os, int ind) const override;
  };

  // A string to print verbatim.
  class TPString : public TPNode {
  public:
    // The string.
    string m_string;

  public:
    TPString(string const &s);
    ~TPString();

    virtual void scan() override;
    virtual void print(PrintState &printState) const override;
    virtual void debugPrint(std::ostream &os, int ind) const override;
  };

  // The sorts of breaks that can appear between strings.
  enum BreakKind {
    // Always a newline.
    BK_NEWLINE_ALWAYS,

    // Newline or space depending on available space on line.
    BK_NEWLINE_OR_SPACE,

    // Newline or nothing depending on available space.
    BK_NEWLINE_OR_NOTHING,

    // Remove one level of indentation from the current line.
    BK_UNINDENT,

    NUM_BREAK_KINDS
  };

  // One of the break kinds.
  class TPBreak : public TPNode {
  public:      // data
    // What kind of break this is.
    BreakKind m_breakKind;

  public:
    TPBreak(BreakKind breakKind)
      : m_breakKind(breakKind)
    {}

    virtual void scan() override;
    virtual void print(PrintState &printState) const override;
    virtual void debugPrint(std::ostream &os, int ind) const override;
  };

  // Manipulation of the sequence stack.
  enum SequenceCommand {
    // Open a new sequence with indentation for subsequent lines.
    SC_BEGIN_INDENT,

    // Open a new sequence without indentation.
    SC_BEGIN_NO_INDENT,

    // Close the innermost sequence.  There must be one (other than the
    // root sequence).
    SC_END,

    NUM_SEQUENCE_COMMANDS
  };

public:      // class data
  // Short names for the breaks.
  static BreakKind const br    = BK_NEWLINE_ALWAYS;
  static BreakKind const sp    = BK_NEWLINE_OR_SPACE;
  static BreakKind const optbr = BK_NEWLINE_OR_NOTHING;
  static BreakKind const und   = BK_UNINDENT;

  // Short names for the commands.
  static SequenceCommand const seq       = SC_BEGIN_INDENT;
  static SequenceCommand const seq_noind = SC_BEGIN_NO_INDENT;
  static SequenceCommand const end       = SC_END;

public:      // data
  // Root of tree to print.
  TPSequence m_root;

  // Stack of open sequences being built.  The top element is what gets
  // added to when operator<< is used.
  std::stack<TPSequence * /*non-owner*/> m_sequenceStack;

public:      // class methods
  // Return "BK_NEWLINE_ALWAYS", etc.
  static char const *toString(BreakKind breakKind);

private:     // methods
  // Append a node to the top-most open sequence.
  void append(TPNode *node);

public:      // methods
  TreePrint();
  ~TreePrint();

  // Add a string.
  TreePrint& operator<< (char const *str);
  TreePrint& operator<< (int i);
  TreePrint& operator<< (char c);
  TreePrint& operator<< (string const &str);

  // Add a break.
  TreePrint& operator<< (BreakKind breakKind);

  // Begin or end a sequence.
  TreePrint& operator<< (SequenceCommand sequenceCommand);

  // True if every sequence opened with SC_BEGIN_XXX has been closed
  // with SC_END.  The client may want to assert this before printing.
  bool allSequencesClosed() const;

  // Print the current tree to 'os'.
  void print(std::ostream &os, int targetWidth = 72);

  // Clear the tree.
  void clear();

  // Debug-print the current tree.
  void debugPrint(std::ostream &os) const;
  void debugPrintCout() const;
};


#endif // SMBASE_TREE_PRINT_H
