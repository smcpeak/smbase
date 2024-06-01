// gdvalue-writer.h
// GDValueWriter class.

// This file is in the public domain.

#ifndef SMBASE_GDVALUE_WRITER_H
#define SMBASE_GDVALUE_WRITER_H

// this dir
#include "gdvalue.h"                   // gdv::GDValueWriteOptions
#include "sm-macros.h"                 // OPEN_NAMESPACE

// libc++
#include <iosfwd>                      // std::ostream


OPEN_NAMESPACE(gdv)


// Manage the process of writing a GDValue to an ostream.
class GDValueWriter {
private:     // instance data
  // Stream to write to.  This is a pointer so we can temporarily
  // reassign it.
  std::ostream *m_os;

  // If true, we are currently checking whether a value can be written
  // onto the current line without exceeding the target line width.
  // Furthermore, we know that 'm_os' is actually a pointer to an
  // CountingOStream created to measure the write.
  bool m_doingSpeculativeWrite;

  // Extra characters that we intend to print after the specifc value
  // under consideration.  These effectively decrease the available
  // space on the current line.
  int m_numExtraChars;

public:      // data
  // Currently active options.
  GDValueWriteOptions m_options;

private:     // methods
  // Write 'container' using the specified optional tag and open and
  // closing delimiters.
  //
  // Return false if we are doing a speculative write and we exceeded
  // the available capacity.
  template <class CONTAINER>
  bool writeContainer(
    CONTAINER const &container,
    char const * NULLABLE tagName,
    char const *openDelim,
    char const *closeDelim);

  // Check if we can write 'value' in the available space on the current
  // line.  'VALUE_TYPE' is either GDValue or GDVMapEntry.
  template <typename VALUE_TYPE>
  bool valueFitsOnLine(
    VALUE_TYPE const &value);

  // Would 'value' plus 'numExtra' chars fit?
  bool valueFitsOnLineWithExtra(
    GDValue const &value,
    int numExtra);

  // Would 'value' fit with one more indentation level?
  bool valueFitsOnLineAfterIndent(
    GDValue const &value);

  // Try to write 'value', returning false if we are currently doing a
  // speculative write and we exceed capacity.
  //
  // If 'forceLineBreaks' is true, then do not consider putting the
  // entire value on one line.
  //
  bool tryWrite(GDValue const &value, bool forceLineBreaks = false);

  // Same as 'tryWrite(GDValue)', but for 'GDVMapEntry'.
  bool tryWrite(GDVMapEntry const &entry, bool forceLineBreaks = false);

  // Write 'str'.  Return false if we are doing a speculative write and
  // we exceed the available capacity.
  bool writeDQString(GDVString const &str);

  // True if we are performing a speculative write, and while doing so,
  // the number of written characters exceeds what is available.
  bool exceededSpeculativeCapacity() const;

  // Write the current indentation amount.
  void writeIndentation();

  // Write a newline and then the current indentation.
  void startNewIndentedLine();

  // Assuming 'value.isContainer()', return the number of characters in
  // the opening delimiter used for its kind.
  int openDelimLength(GDValue const &value) const;

public:      // methods
  // Initialize the writer.
  GDValueWriter(std::ostream &os, GDValueWriteOptions const &options);

  // Get the output stream.
  std::ostream &os()
    { return *m_os; }

  // True if we are using indentation.
  bool usingIndentation() const
    { return m_options.usingIndentation(); }

  // Write 'value' to the output stream at the current indentation level.
  void write(GDValue const &value);
};


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_WRITER_H
