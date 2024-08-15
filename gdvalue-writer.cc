// gdvalue-writer.cc
// Code for gdvalue-writer.

// This file is in the public domain.

#include "gdvalue-writer.h"            // this module

// this dir
#include "counting-ostream.h"          // CountingOStream
#include "gdvalue.h"                   // gdv::GDValue
#include "gdvsymbol.h"                 // gdv::GDVSymbol
#include "overflow.h"                  // safeToInt
#include "save-restore.h"              // SAVE_RESTORE, SET_RESTORE
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "sm-trace.h"                  // INIT_TRACE
#include "string-util.h"               // doubleQuote
#include "stringf.h"                   // stringf
#include "xassert.h"                   // xfailure

// libc++
#include <iostream>                    // std::ostream


OPEN_NAMESPACE(gdv)


INIT_TRACE("gdvalue-writer");


template <class CONTAINER>
bool GDValueWriter::writeContainer(
  CONTAINER const &container,
  std::optional<GDVSymbol> tag,
  ContainerSyntax const &syntax)
{
  TRACE1_SCOPED("writeContainer:"
    " tag=" << (tag? tag->asString() : std::string("(absent)")) <<
    " syntax=" << syntax.m_openDelim << syntax.m_emptyIndicator << syntax.m_closeDelim <<
    " enableIndentation=" << m_options.m_enableIndentation <<
    " indentLevel=" << m_options.m_indentLevel);

  if (tag) {
    os() << *tag;
  }

  os() << syntax.m_openDelim;

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

    ++curIndex;

    if (!tryWrite(val)) {
      TRACE1("writeContainer: bailing after " << curIndex << " items");
      return false;
    }
  }

  if (curIndex == 0) {
    os() << syntax.m_emptyIndicator;
  }

  if (usingIndentation()) {
    --m_options.m_indentLevel;

    if (curIndex > 0) {
      startNewIndentedLine();
    }
  }

  os() << syntax.m_closeDelim;

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


template <typename VALUE_TYPE>
bool GDValueWriter::valueFitsOnLineWithExtra(
  VALUE_TYPE const &value,
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

  // For use inside the `CASE` macro below.
  static ContainerSyntax const sequenceSyntax = { "[", "",  "]" };
  static ContainerSyntax const    tupleSyntax = { "(", "",  ")" };
  static ContainerSyntax const      setSyntax = { "{", "",  "}" };
  static ContainerSyntax const      mapSyntax = { "{", ":", "}" };

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

    case GDVK_SYMBOL: {
      std::string_view name = value.symbolGetName();
      if (GDVSymbol::validUnquotedSymbolName(name)) {
        os() << name;
      }
      else {
        return writeQuotedString(value.symbolGetName(), '`');
      }
      break;
    }

    case GDVK_STRING:
      return writeQuotedString(value.stringGet(), '"');

    #define CASE(KIND, Kind, kind)       \
      case GDVK_##KIND:                  \
        return writeContainer(           \
          value.kind##Get(),             \
          std::nullopt /* tag */,        \
          kind##Syntax);                 \
                                         \
      case GDVK_TAGGED_##KIND:           \
        return writeContainer(           \
          value.kind##Get(),             \
          value.taggedContainerGetTag(), \
          kind##Syntax);

    FOR_EACH_GDV_CONTAINER(CASE)

    #undef CASE
  }

  return !exceededSpeculativeCapacity();
}


/* Case 1: If indentation is disabled, then we print in the most
   compact form:

     key:value

   Case 2: If the key and value both fit on one line, print them on one
   line with a space after the ':':

     key: value

   Case 3: Otherwise, if the key and colon fit on the first line and the
   value fits on the second line after one level of indentation, print
   them on two lines:

     key:
       value

   Case 4: Otherwise, if the value is a container and the key, colon,
   space, and opening delimiter of the container all fit on the first
   line, then put them there, followed by indented elements of the
   container, ending with its closing delimiter on its own line:

     key: (
       value
       value
       value
     )

   Case 5: Otherwise, split the key across lines, then print the value
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

  // The printing case to use.
  enum PrintCase {
    PC_NONE                = 0,
    PC_COMPACT             = 1, // No indentation or space.
    PC_ONE_LINE_WITH_SPACE = 2, // No indentation, but a space.
    PC_TWO_LINES           = 3, // Key on one line, value on next.
    PC_KEY_COLON_OPEN      = 4, // First line has key and container opener.
    PC_KEY_MULTI_LINE      = 5, // Key is spread across multiple lines.
  } printCase = PC_NONE;

  if (!usingIndentation()) {
    // Case 1: Indentation already disabled.
    printCase = PC_COMPACT;
  }
  else if (valueFitsOnLineWithExtra(pair, 1/*space*/)) {
    // Case 2: Both fit on a line with a space.
    printCase = PC_ONE_LINE_WITH_SPACE;
  }
  else if (valueFitsOnLineWithExtra(key, 1) &&
           valueFitsOnLineAfterIndent(value)) {
    // Case 3: Key and value each fit onto their own line.
    printCase = PC_TWO_LINES;
  }
  else if (value.isContainer() &&
           valueFitsOnLineWithExtra(key, 1/*colon*/ + 1/*space*/ +
                                         openDelimLength(value))) {
    // Case 4: Key and start of value fit on first line.
    printCase = PC_KEY_COLON_OPEN;
  }
  else {
    // Case 4: Put the key on multiple lines, and let the recursive call
    // handle the value.
    printCase = PC_KEY_MULTI_LINE;
  }

  TRACE2_SCOPED("tryWrite(GDVMapEntry): printCase=" << printCase);
  xassert(printCase != PC_NONE);

  // Print the key.
  {
    SAVE_RESTORE(m_numExtraChars);
    SAVE_RESTORE(m_options.m_enableIndentation);

    if (printCase == PC_TWO_LINES || printCase == PC_KEY_COLON_OPEN) {
      ++m_numExtraChars;     // For the colon.
      m_options.m_enableIndentation = false;

      if (printCase == PC_KEY_COLON_OPEN) {
        m_numExtraChars += openDelimLength(value);
      }
    }

    if (!tryWrite(key)) {
      return false;
    }
    os() << ':';

    if (printCase == PC_ONE_LINE_WITH_SPACE ||
        printCase == PC_KEY_COLON_OPEN) {
      os() << ' ';
    }
  }

  // Value.
  {
    SAVE_RESTORE(m_options.m_indentLevel);
    SAVE_RESTORE(m_options.m_enableIndentation);

    if (usingIndentation() && printCase != PC_KEY_COLON_OPEN) {
      // In case 3, the value container itself is effectively not
      // indented; the elements of the container will be indented
      // because of what the container does, but its opening delimiter
      // is on the same line as the key, and its closing delimiter is
      // lined up vertically with the start of the key.
      ++m_options.m_indentLevel;
    }
    if (printCase == PC_COMPACT || printCase == PC_ONE_LINE_WITH_SPACE) {
      m_options.m_enableIndentation = false;
    }
    if (printCase == PC_TWO_LINES || printCase == PC_KEY_MULTI_LINE) {
      startNewIndentedLine();
    }
    bool const forceLineBreaks = (printCase==PC_KEY_COLON_OPEN);
    if (!tryWrite(value, forceLineBreaks)) {
      return false;
    }
  }

  return true;
}


bool GDValueWriter::writeQuotedString(std::string_view str, char delim)
{
  os() << delim;

  for (char c : str) {
    writeOneQuotedStringChar(os(), c, delim,
      m_options.m_useUndelimitedHexEscapes);

    if (exceededSpeculativeCapacity()) {
      return false;
    }
  }

  os() << delim;
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


int GDValueWriter::openDelimLength(GDValue const &value) const
{
  // Either '{{' for a set, or '[' or '(' or '{' for the others.
  int ret = value.isSet()? 2 : 1;

  if (value.isTaggedContainer()) {
    ret += safeToInt(value.taggedContainerGetTag().size());
  }

  return ret;
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


STATICDEF void GDValueWriter::writeOneQuotedStringChar(
  std::ostream &os,
  char c,
  char delim,
  bool useUndelimitedHexEscapes)
{
  switch (c) {
    case '"':
    case '`':
      // Quote a delimiter only if it is the one we are using.
      if (c == delim) {
        os << "\\";
      }
      os << c;
      break;

    case '\\':
      os << "\\\\";
      break;

    case '\b':
      os << "\\b";
      break;

    case '\f':
      os << "\\f";
      break;

    case '\n':
      os << "\\n";
      break;

    case '\r':
      os << "\\r";
      break;

    case '\t':
      os << "\\t";
      break;

    default:
      if ((unsigned char)c < 0x20) {
        if (useUndelimitedHexEscapes) {
          os << stringf("\\u%04X", (int)(unsigned char)c);
        }
        else {
          os << stringf("\\u{%X}", (int)(unsigned char)c);
        }
      }
      else {
        // I'm not using numeric escapes for anything except
        // non-printable characters since, ideally, the producer and
        // consumer both speak UTF-8 fluently.
        //
        // Note that UTF-8 code units will be written one at a time
        // by this line.
        os << c;
      }
      break;
  }
}


CLOSE_NAMESPACE(gdv)


// EOF
