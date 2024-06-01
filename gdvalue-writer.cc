// gdvalue-writer.cc
// Code for gdvalue-writer.

// This file is in the public domain.

#include "gdvalue-writer.h"            // this module

// this dir
#include "counting-ostream.h"          // CountingOStream
#include "gdvalue.h"                   // gdv::GDValue
#include "gdvsymbol.h"                 // gdv::GDVSymbol
#include "save-restore.h"              // SAVE_RESTORE, SET_RESTORE
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "sm-trace.h"                  // INIT_TRACE
#include "string-utils.h"              // doubleQuote
#include "stringf.h"                   // stringf
#include "xassert.h"                   // xfailure

// libc++
#include <iostream>                    // std::ostream


OPEN_NAMESPACE(gdv)


INIT_TRACE("gdvalue-writer");


// True if 't' is a GDVPair whose first element is a GDVMap.
//
// In the general case, no.
template <class T>
static bool isPairWithMapAsFirstElement(T const & /*t*/)
{
  return false;
}


// Specialize for map entries.
template <>
bool isPairWithMapAsFirstElement(GDVMapEntry const &entry)
{
  return entry.first.isMap();
}


template <class CONTAINER>
bool GDValueWriter::writeContainer(
  CONTAINER const &container,
  char const *openDelim,
  char const *closeDelim)
{
  TRACE1_SCOPED("writeContainer:"
    " delims=" << openDelim << closeDelim <<
    " enableIndentation=" << m_options.m_enableIndentation <<
    " indentLevel=" << m_options.m_indentLevel);

  os() << openDelim;

  if (exceededSpeculativeCapacity()) {
    TRACE1("writeContainer: bailing after openDelim");
    return false;
  }

  // Make sure that the indentation level is restored even if we bail
  // out early.
  SAVE_RESTORE(m_options.m_indentLevel);

  if (usingIndentation()) {
    // If we start a new line, we will indent one more level than the
    // previous new line.
    ++m_options.m_indentLevel;
  }

  int curIndex = 0;

  // This function is a template so this iteration is generic w.r.t.
  // the container type.
  for (auto const &val : container) {
    if (usingIndentation()) {
      startNewIndentedLine();
    }
    else if (curIndex > 0) {
      os() << ' ';
    }
    else if (isPairWithMapAsFirstElement(val)) {
      // We are writing the first entry of a map, and the first key is
      // also a map.  We need to emit a space in order to separate the
      // opening braces of the two maps, since otherwise they will be
      // treated as the start of a set.
      os() << ' ';
    }

    ++curIndex;

    if (!tryWrite(val)) {
      TRACE1("writeContainer: bailing after " << curIndex << " items");
      return false;
    }
  }

  if (usingIndentation()) {
    --m_options.m_indentLevel;

    if (curIndex > 0) {
      startNewIndentedLine();
    }
  }

  os() << closeDelim;

  if (exceededSpeculativeCapacity()) {
    TRACE1("writeContainer: bailing after closeDelim");
    return false;
  }

  TRACE1("writeContainer: returning true");
  return true;
}


template <typename VALUE_TYPE>
bool GDValueWriter::valueFitsOnLine(
  VALUE_TYPE const &value)
{
  TRACE2_SCOPED("valueFitsOnLine:"
    " type=" << (sizeof(value) == sizeof(GDVMapEntry)? "entry" : "value") <<
    " numExtraChars=" << m_numExtraChars);

  // For this to be called, we must be considering using indentation.
  xassertPrecondition(usingIndentation());

  // Disable indentation for this check.
  SET_RESTORE(m_options.m_enableIndentation, false);

  // Point write operations at a temporary stream whose contents will be
  // discarded.
  CountingOStream cos;
  SET_RESTORE(m_os, &cos);

  // Indicate that we are performing a speculative write so the check
  // for exceeding capacity knows that 'm_os' points at a
  // CountingOStream.
  SET_RESTORE(m_doingSpeculativeWrite, true);

  // Speculatively write to the temporary stream, stopping if we exceed
  // the target line width.
  if (tryWrite(value)) {
    // The write succeeded without going over.
    TRACE2("valueFitsOnLine returning true");
    return true;
  }
  else {
    // Hit the limit.
    TRACE2("valueFitsOnLine returning false");
    return false;
  }
}


bool GDValueWriter::valueFitsOnLineWithExtra(
  GDValue const &value,
  int numExtra)
{
  SAVE_RESTORE(m_numExtraChars);
  m_numExtraChars += numExtra;
  return valueFitsOnLine(value);
}


bool GDValueWriter::valueFitsOnLineAfterIndent(
  GDValue const &value)
{
  SAVE_RESTORE(m_options.m_indentLevel);
  ++m_options.m_indentLevel;
  return valueFitsOnLine(value);
}


bool GDValueWriter::tryWrite(GDValue const &value,
                             bool forceLineBreaks)
{
  TRACE1_SCOPED("tryWrite(GDValue):"
    " kind=" << toString(value.getKind()) <<
    " forceLineBreaks=" << forceLineBreaks <<
    " numExtraChars=" << m_numExtraChars <<
    " enableIndentation=" << m_options.m_enableIndentation <<
    " indentLevel=" << m_options.m_indentLevel);

  SAVE_RESTORE(m_options.m_enableIndentation);
  if (!forceLineBreaks &&
      usingIndentation() &&
      valueFitsOnLine(value)) {
    // Disable the use of indentation.
    m_options.m_enableIndentation = false;
  }

  switch (value.getKind()) {
    default:
      xfailureInvariant("invalid kind");

    case GDVK_INTEGER:
      if (m_options.m_writeLargeIntegersAsDecimal) {
        os() << value.largeIntegerGet();
      }
      else {
        os() << value.largeIntegerGet().toHexString();
      }
      break;

    case GDVK_SMALL_INTEGER:
      os() << value.smallIntegerGet();
      break;

    case GDVK_SYMBOL:
      os() << value.symbolGet().getSymbolName();
      break;

    case GDVK_STRING:
      return writeDQString(value.stringGet());

    case GDVK_SEQUENCE:
      return writeContainer(
        value.sequenceGet(),
        "[",
        "]");

    case GDVK_SET:
      // TODO: Explain the choice of syntax in the design doc.
      return writeContainer(
        value.setGet(),
        "{{",
        "}}");

    case GDVK_MAP:
      return writeContainer(
        value.mapGet(),
        "{",
        "}");
  }

  return !exceededSpeculativeCapacity();
}


/* Case 1: If (a) indentation is disabled, or (b) the key and value both
   fit on one line, print them on one line:

     key:value

   Case 2: Otherwise, if the key and colon fit on the first line and the
   value fits on the second line after one level indentation, print them
   on two lines:

     key:
       value

   Case 3: Otherwise, if the value is a container and the key, colon,
   and opening delimiter of the container all fit on the first line,
   then put them there, followed by indented elements of the container,
   ending with its closing delimiter on its own line:

     key:(
       value
       value
       value
     )

   Case 4: Otherwise, split the key across lines, then print the value
   on the next line, indented if necessary:

     (
       key
       key
       key
     ):
       value (possibly indented)
*/
bool GDValueWriter::tryWrite(GDVMapEntry const &pair,
                             bool /*forceLineBreaks*/)
{
  TRACE1_SCOPED("tryWrite(GDVMapEntry):"
    " numExtraChars=" << m_numExtraChars <<
    " enableIndentation=" << m_options.m_enableIndentation <<
    " indentLevel=" << m_options.m_indentLevel);

  GDValue const &key   = pair.first;
  GDValue const &value = pair.second;

  // The case to use, from among those described in the comment above.
  int printCase;

  if (!usingIndentation()) {
    // Case 1a: Indentation already disabled.
    printCase = 1;
  }
  else if (valueFitsOnLine(pair)) {
    // Case 1b: Both fit, so disable indentation.
    printCase = 1;
  }
  else if (valueFitsOnLineWithExtra(key, 1) &&
           valueFitsOnLineAfterIndent(value)) {
    // Case 2: Key and value each fit onto their own line.
    printCase = 2;
  }
  else if (isContainer(value) &&
           valueFitsOnLineWithExtra(key, 1 + openDelimLength(value))) {
    // Case 3: Key and start of value fit on first line.
    printCase = 3;
  }
  else {
    // Case 4: Put the key on multiple lines, and let the recursive call
    // handle the value.
    printCase = 4;
  }

  TRACE2_SCOPED("tryWrite(GDVMapEntry): printCase=" << printCase);

  // Print the key.
  {
    SAVE_RESTORE(m_numExtraChars);
    SAVE_RESTORE(m_options.m_enableIndentation);

    if (printCase == 2 || printCase == 3) {
      ++m_numExtraChars;     // For the colon.
      m_options.m_enableIndentation = false;

      if (printCase == 3) {
        m_numExtraChars += openDelimLength(value);
      }
    }

    if (!tryWrite(key)) {
      return false;
    }
    os() << ':';
  }

  // Value.
  {
    SAVE_RESTORE(m_options.m_indentLevel);
    SAVE_RESTORE(m_options.m_enableIndentation);

    if (usingIndentation() && printCase != 3) {
      // In case 3, the value container itself is effectively not
      // indented; the elements of the container will be indented
      // because of what the container does, but its opening delimiter
      // is on the same line as the key, and its closing delimiter is
      // lined up vertically with the start of the key.
      ++m_options.m_indentLevel;
    }
    if (printCase == 1) {
      m_options.m_enableIndentation = false;
    }
    if (printCase == 2 || printCase == 4) {
      startNewIndentedLine();
    }
    bool const forceLineBreaks = (printCase==3);
    if (!tryWrite(value, forceLineBreaks)) {
      return false;
    }
  }

  return true;
}


bool GDValueWriter::writeDQString(GDVString const &str)
{
  os() << '"';

  for (char c : str) {
    switch (c) {
      case '"':
        os() << "\\\"";
        break;

      case '\\':
        os() << "\\\\";
        break;

      case '\b':
        os() << "\\b";
        break;

      case '\f':
        os() << "\\f";
        break;

      case '\n':
        os() << "\\n";
        break;

      case '\r':
        os() << "\\r";
        break;

      case '\t':
        os() << "\\t";
        break;

      default:
        if ((unsigned char)c < 0x20) {
          os() << stringf("\\u%04X", (int)(unsigned char)c);
        }
        else {
          // I'm not using numeric escapes for anything except
          // non-printable characters since, ideally, the producer and
          // consumer both speak UTF-8 fluently.
          //
          // Note that UTF-8 code units will be written one at a time
          // by this line.
          os() << c;
        }
        break;
    }

    if (exceededSpeculativeCapacity()) {
      return false;
    }
  }

  os() << '"';
  if (exceededSpeculativeCapacity()) {
    return false;
  }

  return true;
}


bool GDValueWriter::exceededSpeculativeCapacity() const
{
  if (!m_doingSpeculativeWrite) {
    return false;
  }

  auto *cos = static_cast<CountingOStream*>(m_os);
  std::size_t numWritten = cos->getCount();
  int capacity = m_options.lineCapacity() - m_numExtraChars;

  bool ret = capacity < 0 ||
             static_cast<std::size_t>(capacity) < numWritten;

  TRACE2("exceededSpeculativeCapacity:"
    " numWritten=" << numWritten <<
    " numExtraChars=" << m_numExtraChars <<
    " capacity=" << capacity <<
    " ret=" << ret);

  return ret;
}


void GDValueWriter::writeIndentation()
{
  int const ct = m_options.currentIndentationSpaceCount();
  for (int i=0; i < ct; ++i) {
    os() << ' ';
  }
}


void GDValueWriter::startNewIndentedLine()
{
  os() << '\n';
  writeIndentation();
}


bool GDValueWriter::isContainer(GDValue const &value) const
{
  switch (value.getKind()) {
    case GDVK_SEQUENCE:
    case GDVK_SET:
    case GDVK_MAP:
      return true;

    default:
      return false;
  }
}


int GDValueWriter::openDelimLength(GDValue const &value) const
{
  switch (value.getKind()) {
    default:
      xfailurePrecondition("not a container");

    case GDVK_SEQUENCE:
    case GDVK_MAP:
      return 1;    // '(' or '{'

    case GDVK_SET:
      return 2;    // '{{'
  }
}


GDValueWriter::GDValueWriter(std::ostream &os,
                             GDValueWriteOptions const &options)
  : m_os(&os),
    m_doingSpeculativeWrite(false),
    m_numExtraChars(0),
    m_options(options)
{}


void GDValueWriter::write(GDValue const &value)
{
  // Call the private method that returns 'bool'.  In this context, it
  // will always return true.
  bool res = tryWrite(value);
  xassert(res);
}


CLOSE_NAMESPACE(gdv)


// EOF
