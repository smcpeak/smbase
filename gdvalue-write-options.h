// gdvalue-write-options.h
// GDValueWriteOptions class.

// This file is in the public domain.

#ifndef SMBASE_GDVALUE_WRITE_OPTIONS_H
#define SMBASE_GDVALUE_WRITE_OPTIONS_H

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE


OPEN_NAMESPACE(gdv)


// Options for how to write a GDValue as text.
class GDValueWriteOptions {
public:      // class data
  // Default value of 'm_spacesPerIndentLevel'.  Initially 2.
  static int s_defaultSpacesPerIndentLevel;

  // Default value of 'm_targetLineWidth'.  Initially 72.
  static int s_defaultTargetLineWidth;

public:      // instance data
  // When true, we will use newlines and indentation to show the
  // structure and to try to stay within the target line width.
  // Otherwise, a compact, single-line format is used.  Initially false.
  bool m_enableIndentation;

  // When true, large integers will be written using decimal digits
  // rather than hexadecimal.  Initially false.
  bool m_writeLargeIntegersAsDecimal;

  // When true, and we want to write a hex escape "universal character"
  // in a string or symbol, use the undelimited form "\u0000" instead of
  // the delimited form "\u{0}".  Initially false.
  bool m_useUndelimitedHexEscapes;

  // Current indentation level.  When we start a new line, we indent
  // 'm_indentLevel * SPACES_PER_INDENT_LEVEL' spaces.  Initially 0.
  int m_indentLevel;

  // Number of spaces to print for each indentation level.  Initially
  // set to 's_defaultSpacesPerIndentLevel'.
  int m_spacesPerIndentLevel;

  // Target line width when using indentation.  Initially set to
  // 's_defaultTargetLineWidth'.  If this is set to zero, then every
  // possible line break will be taken.
  int m_targetLineWidth;

public:      // methods
  GDValueWriteOptions()
    : m_enableIndentation(false),
      m_writeLargeIntegersAsDecimal(false),
      m_useUndelimitedHexEscapes(false),
      m_indentLevel(0),
      m_spacesPerIndentLevel(s_defaultSpacesPerIndentLevel),
      m_targetLineWidth(s_defaultTargetLineWidth)
  {}

  GDValueWriteOptions(GDValueWriteOptions const &obj) = default;
  GDValueWriteOptions &operator=(GDValueWriteOptions const &obj) = default;

  // Chainable setters.
  GDValueWriteOptions &setEnableIndentation(bool b)
    { m_enableIndentation = b; return *this; }
  GDValueWriteOptions &setWriteLargeIntegersAsDecimal(bool b)
    { m_writeLargeIntegersAsDecimal = b; return *this; }
  GDValueWriteOptions &setUseUndelimitedHexEscapes(bool b)
    { m_useUndelimitedHexEscapes = b; return *this; }
  GDValueWriteOptions &setIndentLevel(int newLevel)
    { m_indentLevel = newLevel; return *this; }
  GDValueWriteOptions &setSpacesPerIndentLevel(int newSpacesPer)
    { m_spacesPerIndentLevel = newSpacesPer; return *this; }
  GDValueWriteOptions &setTargetLineWidth(int newTarget)
    { m_targetLineWidth = newTarget; return *this; }

  // True if we are using indentation.
  bool usingIndentation() const
    { return m_enableIndentation; }

  // Number of spaces to print for the current indentation level.
  int currentIndentationSpaceCount() const
    { return m_indentLevel * m_spacesPerIndentLevel; }

  // Number of characters that fit on the current line between the end
  // of the current indentation and the target maximum width.  It may be
  // negative if the current indentation already exceeds the target
  // width, which could happen when printing deeply nested structures.
  int lineCapacity() const;
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_WRITE_OPTIONS_H
