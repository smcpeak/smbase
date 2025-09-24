// gdvalue-reader.cc
// Code for gdvalue-reader.h.

// This file is in the public domain.

#include "gdvalue-reader.h"            // this module

#include "smbase/codepoint.h"          // isWhitespace, decodeRadixIndicatorLetter, isASCIIRadixDigit
#include "smbase/exc.h"                // THROW
#include "smbase/file-line-col.h"      // FileLineCol
#include "smbase/gdv-binary64-float.h" // gdv::GDVBinary64Float
#include "smbase/gdv-symbol.h"         // GDVSymbol
#include "smbase/gdvalue.h"            // GDValue
#include "smbase/line-col.h"           // LineCol
#include "smbase/sm-env.h"             // smbase::envAsBool
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/string-util.h"        // possiblyTruncatedWithEllipsis
#include "smbase/stringf.h"            // stringf
#include "smbase/utf8-writer.h"        // smbase::UTF8Writer
#include "smbase/xassert.h"            // xassert

#include <algorithm>                   // std::max
#include <optional>                    // std::optional
#include <string>                      // std::string
#include <string_view>                 // std::string_view
#include <utility>                     // std::move

using namespace smbase;


OPEN_NAMESPACE(gdv)


// ------------------- GDValueSourceLocationAndChar --------------------
// ---- create-tuple-class: definitions for GDValueSourceLocationAndChar
/*AUTO_CTC*/ GDValueSourceLocationAndChar::GDValueSourceLocationAndChar(
/*AUTO_CTC*/   GDValueSourceLocation const &loc,
/*AUTO_CTC*/   int c)
/*AUTO_CTC*/   : IMEMBFP(loc),
/*AUTO_CTC*/     IMEMBFP(c)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ GDValueSourceLocationAndChar::GDValueSourceLocationAndChar(GDValueSourceLocationAndChar const &obj) noexcept
/*AUTO_CTC*/   : DMEMB(m_loc),
/*AUTO_CTC*/     DMEMB(m_c)
/*AUTO_CTC*/ {}
/*AUTO_CTC*/
/*AUTO_CTC*/ GDValueSourceLocationAndChar &GDValueSourceLocationAndChar::operator=(GDValueSourceLocationAndChar const &obj) noexcept
/*AUTO_CTC*/ {
/*AUTO_CTC*/   if (this != &obj) {
/*AUTO_CTC*/     CMEMB(m_loc);
/*AUTO_CTC*/     CMEMB(m_c);
/*AUTO_CTC*/   }
/*AUTO_CTC*/   return *this;
/*AUTO_CTC*/ }
/*AUTO_CTC*/


// --------------------------- GDValueReader ---------------------------
GDValueReader::GDValueReader(
  std::istream &is,
  std::optional<std::string> fileNameOpt)
:
  Reader(is, fileNameOpt),
  m_fileIndex(fileNameOpt?
    GDValueSourceLocation::fileIndexOfName(*fileNameOpt) : 0)
{}


GDValueReader::~GDValueReader()
{}


GDValueSourceLocation GDValueReader::gdvLocOffset(int columnOffset) const
{
  return GDValueSourceLocation(
    m_fileIndex,

    m_location.m_lc.m_line,

    // The column offset can be 0 if we read a newline and then put it
    // back.  This happens for example in
    // `readNextSymbolOrTaggedContainer` when a symbol is immediately
    // followed by a newline.  When this happens, we will not end up
    // using the location for anything, but it is awkward to avoid
    // computing one.  So, use `std::max` to force the column to be
    // valid for `GDValueSourceLocation`.
    //
    // Ultimately, this is a consequence of the somewhat weird way my
    // `Reader` class works; there are some comments on its declaration
    // explaining the problems.  I think if that were cleaned up, the
    // problem here would be solved too.
    //
    std::max(1, m_location.m_lc.m_column + columnOffset));
}


GDValueSourceLocation GDValueReader::gdvLocPrevChar() const
{
  return gdvLocOffset(-1);
}


GDValueSourceLocation GDValueReader::gdvLoc() const
{
  return gdvLocOffset(0);
}


GDValueSourceLocationAndChar GDValueReader::readLocChar()
{
  GDValueSourceLocation loc = gdvLoc();
  return GDValueSourceLocationAndChar(loc, readChar());
}


void GDValueReader::readEOFOrErr()
{
  int c = readCharAfterWhitespaceAndComments();
  if (c != eofCode()) {
    unexpectedCharErr(c, "looking for the end of a file that should only have one value");
  }
}


// For now, this method is just a slight convenience, and relies on the
// location tracking done by the underlying `Reader`.  But I'm thinking
// at some point I might do all location tracking in `GDValueReader`, in
// which case this method will be more useful.
void GDValueReader::unexpectedLocCharErr(
  GDValueSourceLocationAndChar const &locChar,
  char const *lookingFor)
{
  unexpectedCharErr(locChar.m_c, lookingFor);
}


bool GDValueReader::isAllowedAfterValue(int c)
{
  if (c == eofCode()) {
    return true;
  }

  switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case ',':
    case '}':
    case ']':
    case ')':
    case ':':
      return true;

    default:
      return false;
  }
}


void GDValueReader::checkAfterValueOrErr(int c)
{
  if (!isAllowedAfterValue(c)) {
    inCtxUnexpectedCharErr(c,
      "after a value; every value must be followed by EOF, whitespace, "
      "',', ':', ']', ')', or '}'");
  }
}


void GDValueReader::putbackAfterValueOrErr(int c)
{
  checkAfterValueOrErr(c);
  putback(c);
}


int GDValueReader::readCharAfterWhitespaceAndComments()
{
  while (true) {
    int c = readChar();
    if (c == eofCode()) {
      return c;
    }

    switch (c) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
      case ',':
        break;

      case '/':
        // Start of comment.
        c = readChar();
        if (c == '/') {
          // "//" comment, skip until EOL.
          while (true) {
            c = readChar();
            if (c == eofCode()) {
              return c;
            }
            else if (c == '\n') {
              break;    // End of "//" comment.
            }

            // Not EOF, not newline, so keep skipping.
          }
        }
        else if (c == '*') {
          // "/*" comment, skip to *corresponding* (balanced) "*/".
          skipCStyleComment(0 /*nestingDepth*/);
        }
        else {
          unexpectedCharErr(c, "looking for character after '/'");
        }
        break;

      default:
        // Not comment or whitespace.
        return c;
    }
  }

  // Not reached.
}


GDValueSourceLocationAndChar
GDValueReader::readLocCharAfterWhitespaceAndComments()
{
  int c = readCharAfterWhitespaceAndComments();

  // The use of `gdvLocPrevChar` is inelegant, but it means I can avoid
  // individually tracking the locations while skipping whitespace.
  return GDValueSourceLocationAndChar(gdvLocPrevChar(), c);
}


void GDValueReader::skipCStyleComment(int nestingDepth)
{
  // Number of child "/*...*/" comments of this one.
  int childComments = 0;

  // Read a character and bail on EOF, reporting the current nesting
  // depth and child comment count.
  auto readCommentCharNoEOFOrErr = [&]() -> int {
    int c = readChar();
    if (c == eofCode()) {
      std::ostringstream oss;
      oss << "inside \"/*\" comment, ";
      if (nestingDepth > 0) {
        oss << "nested inside " << nestingDepth
            << " other comments of the same kind, ";
      }
      if (childComments > 0) {
        oss << "which contains " << childComments << " child comments, ";
      }
      oss << "looking for corresponding \"*/\"";

      unexpectedCharErr(c, oss.str().c_str());
    }
    return c;
  };

  while (true) {
    int c = readCommentCharNoEOFOrErr();
    switch (c) {
      case '/':
        c = readCommentCharNoEOFOrErr();
        if (c == '*') {
          // Recursively skip a nested comment.
          ++childComments;
          skipCStyleComment(nestingDepth + 1);
        }
        else if (c == '/') {
          // Note: A "//" inside a "/*...*/" comment does *not* cause
          // closing delimiters of the latter to be ignored.
        }
        break;

      case '*':
      checkAfterStar:
        c = readCommentCharNoEOFOrErr();
        if (c == '/') {
          // Done with this comment.
          return;
        }
        else if (c == '*') {
          // We need to check whether there is a slash after *this*
          // star.  (Yes, this can be done with a 'while' loop.  I think
          // the 'goto' is clearer in this instance.)
          goto checkAfterStar;
        }
        break;

      default:
        break;
    }
  }

  // Not reached.
}


GDValue GDValueReader::readSequenceAfterFirstValue(
  GDValueSourceLocationAndChar const &openingDelim,
  GDValue &&firstValue)
{
  GDValue ret(GDVK_SEQUENCE, openingDelim.m_loc);

  ret.sequenceAppend(std::move(firstValue));

  while (true) {
    std::optional<GDValue> next = readNextValue();
    if (!next) {
      readCharOrErr(']', "looking for ']' at end of sequence");
      return ret;
    }

    ret.sequenceAppend(std::move(*next));
  }

  // Not reached.
}


GDValue GDValueReader::readNextTuple(
  GDValueSourceLocationAndChar const &openingDelim)
{
  GDValue ret(GDVK_TUPLE, openingDelim.m_loc);

  while (true) {
    std::optional<GDValue> next = readNextValue();
    if (!next) {
      readCharOrErr(')', "looking for ')' at end of tuple");
      return ret;
    }

    ret.tupleAppend(std::move(*next));
  }

  // Not reached.
}


GDValue GDValueReader::readNextPossibleMap(
  GDValueSourceLocationAndChar const &openingDelim)
{
  bool const ordered = (openingDelim.m_c == '[');
  char const closingDelim = (ordered? ']' : '}');

  // Check first character after opening delimiter for something special.
  int firstChar = readCharAfterWhitespaceAndComments();

  if (firstChar == closingDelim) {
    // Empty set or sequence.
    if (ordered) {
      return GDValue(GDVK_SEQUENCE, openingDelim.m_loc);
    }
    else {
      return GDValue(GDVK_SET, openingDelim.m_loc);
    }
  }

  if (firstChar == ':') {
    // Empty map or ordered map; but need to confirm the following
    // closing delimiter.
    processCharOrErr(readCharAfterWhitespaceAndComments(), closingDelim,
      ordered?
        "looking for ']' after ':' of empty ordered map" :
        "looking for '}' after ':' of empty map");
    return GDValue(ordered? GDVK_ORDERED_MAP : GDVK_MAP,
                   openingDelim.m_loc);
  }

  // Put back the first character and read the next value.
  putback(firstChar);
  std::optional<GDValue> firstValue = readNextValue();
  if (!firstValue) {
    unexpectedCharErr(readChar(), ordered?
      "looking for a value after '['" :
      "looking for a value after '{'");
  }

  // Check the character after that value.
  int charAfterValue = readCharAfterWhitespaceAndComments();
  if (charAfterValue == ':') {
    // Commit to the map or ordered map interpretation.
    return readPossiblyOrderedMapAfterFirstKey(
      openingDelim, ordered, std::move(*firstValue));
  }
  else {
    putback(charAfterValue);
    if (ordered) {
      return readSequenceAfterFirstValue(
        openingDelim, std::move(*firstValue));
    }
    else {
      return readSetAfterFirstValue(
        openingDelim, std::move(*firstValue));
    }
  }
}


GDValue GDValueReader::readSetAfterFirstValue(
  GDValueSourceLocationAndChar const &openingDelim,
  GDValue &&firstValue)
{
  GDValue ret(GDVK_SET, openingDelim.m_loc);
  ret.setInsert(std::move(firstValue));

  while (true) {
    std::optional<GDValue> next = readNextValue();
    if (!next) {
      readCharOrErr('}', "looking for '}' at end of set");
      return ret;
    }

    ret.setInsert(std::move(*next));
  }
}


GDValue GDValueReader::readPossiblyOrderedMapAfterFirstKey(
  GDValueSourceLocationAndChar const &openingDelim,
  bool ordered,
  GDValue &&firstKey)
{
  char const closingDelim = (ordered? ']' : '}');

  GDValue ret(ordered? GDVK_ORDERED_MAP : GDVK_MAP,
              openingDelim.m_loc);

  // Read the first value.
  std::optional<GDValue> firstValue = readNextValue();
  if (!firstValue) {
    unexpectedCharErr(readChar(), ordered?
      "looking for value after ':' in ordered map entry" :
      "looking for value after ':' in map entry");
  }
  ret.mapSetValueAt(std::move(firstKey), std::move(*firstValue));

  // Read second and later key/value entries.
  while (true) {
    // Skip leading whitespace.
    int firstKeyChar = readCharAfterWhitespaceAndComments();

    // Save this location as the key location in case we need to report
    // a duplicate key error below.
    LineCol keyLC = m_location.getLineCol();

    // Put the first key character back so 'readNextValue' will see it.
    putback(firstKeyChar);

    // Read the key.
    std::optional<GDValue> key = readNextValue();
    if (!key) {
      readCharOrErr(closingDelim, ordered?
        "looking for ']' at end of ordered map" :
        "looking for '}' at end of map");
      return ret;
    }

    int colon = readCharAfterWhitespaceAndComments();

    processCharOrErr(colon, ':', ordered?
      "looking for ':' in ordered map entry" :
      "looking for ':' in map entry");

    // Read the value.
    std::optional<GDValue> value = readNextValue();
    if (!value) {
      unexpectedCharErr(readChar(), ordered?
        "looking for value after ':' in ordered map entry" :
        "looking for value after ':' in map entry");
    }

    if (ret.mapContains(*key)) {
      // Get the key as GDVN.
      std::string keyAsString =
        possiblyTruncatedWithEllipsis(
          key->asString(), 60);

      // Use the location we saved before.
      FileLineCol loc(m_location);
      loc.setLineCol(keyLC);

      locErr(loc, stringb("Duplicate " << (ordered? "ordered " : "") <<
                          "map key: " << keyAsString));
    }

    ret.mapSetValueAt(std::move(*key), std::move(*value));
  }

  // Not reached.
}


GDValue GDValueReader::readNextDQString(
  GDValueSourceLocationAndChar const &openingDelim)
{
  return GDValue(readNextQuotedStringContents(openingDelim.m_c),
                 openingDelim.m_loc);
}


std::string GDValueReader::readNextQuotedStringContents(int delim)
{
  std::ostringstream oss;
  UTF8Writer utf8Writer(oss);

  char const *lookingForCharAfterBackslash =
    delim == '"'?
      "looking for character after '\\' in double-quoted string" :
      "looking for character after '\\' in backtick-quoted symbol";

  while (true) {
    int c = readNotEOFCharOrErr(
      delim == '"'?
        "looking for closing '\"' in double-quoted string" :
        "looking for closing '`' in backtick-quoted symbol");

    if (c == delim) {
      break;
    }

    if (c == '\\') {
      c = readNotEOFCharOrErr(lookingForCharAfterBackslash);

      // Interpret what follows the backslash.
      switch (c) {
        // Characters that denote themselves.
        case '"':
        case '\'':
        case '`':
        case '\\':
        case '/':
          oss << (char)c;
          break;

        case 'b':
          oss << '\b';
          break;

        case 'f':
          oss << '\f';
          break;

        case 'n':
          oss << '\n';
          break;

        case 'r':
          oss << '\r';
          break;

        case 't':
          oss << '\t';
          break;

        case 'u':
          // Store the decoded value as UTF-8.
          utf8Writer.writeCodePoint(readNextUniversalCharacterEscape());
          break;

        default:
          unexpectedCharErr(c, lookingForCharAfterBackslash);
          break;
      } // switch(c) after backslash
    } // if(backslash)

    else /* not backslash, double-quote, or EOF */ {
      // Ordinary character.
      oss << (char)c;
    }
  } // while(true)

  return oss.str();
}


int GDValueReader::readNextUniversalCharacterEscape()
{
  // Check for "\u{N+}".
  int c = readChar();
  if (c == '{') {
    return readNextDelimitedCharacterEscape();
  }
  putback(c);

  // Decode hex.
  int decoded = readNextU4Escape();

  if (isHighSurrogate(decoded)) {
    try {
      // This should be followed by the other half of a surrogate
      // pair.
      readCharOrErr('\\', "expecting '\\'");
      readCharOrErr('u', "expecting 'u' after '\\'");
      int decoded2 = readNextU4Escape();

      if (isLowSurrogate(decoded2)) {
        decoded = decodeSurrogatePair(decoded, decoded2).value();
      }
      else {
        err(stringf(
          "Expected low surrogate in [U+DC00,U+DFFF], "
          "but instead found \"\\u%04X\".",
          decoded2));
      }
    }
    catch (XReader &e) {
      // We do not compute the more detailed context above
      // because we do not yet know we will report an error.
      e.prependErrorContext(stringf(
        "After high surrogate \"\\u%04X\"", decoded));
      throw;
    }
  }
  else if (isLowSurrogate(decoded)) {
    err(stringf(
      "Found low surrogate \"\\u%04X\" that is not preceded by "
      "a high surrogate in [U+D800,U+DBFF].",
      decoded));
  }

  return decoded;
}


int GDValueReader::readNextU4Escape()
{
  int decoded = 0;
  for (int i=0; i < 4; ++i) {
    int c = readChar();
    if (isASCIIHexDigit(c)) {
      // This can't overflow because there are only four digits (and I
      // am assuming that `int` is at least 32 bits wide).
      decoded = decoded*16 + decodeASCIIHexDigit(c);
    }
    else {
      unexpectedCharErr(c, "looking for digits in \"\\u\" escape sequence");
    }
  }

  return decoded;
}


int GDValueReader::readNextDelimitedCharacterEscape()
{
  // There must always be at least one hex digit.
  int c = readChar();
  if (!isASCIIHexDigit(c)) {
    unexpectedCharErr(c,
      R"(looking for hex digit immediately after "\u{")");
  }

  int decoded = decodeASCIIHexDigit(c);

  while (true) {
    c = readChar();
    if (c == '}') {
      break;
    }
    else if (!isASCIIHexDigit(c)) {
      unexpectedCharErr(c,
        R"(looking for hex digit or '}' after "\u{")");
    }

    // This won't overflow because we would trip the "value too large"
    // check first.
    decoded = decoded*16 + decodeASCIIHexDigit(c);

    if (decoded > 0x10FFFF) {
      err(R"(value is larger than 0x10FFFF in "\u{N+}" escape sequence)");
    }
  }

  return decoded;
}


GDValue GDValueReader::readNextNumber(
  GDValueSourceLocationAndChar const &firstChar)
{
  // We will collect all of the characters of the number here before
  // interpreting them as a number.
  std::vector<char> digits;

  // In the steady state, `c` has the next character to process.
  int c = firstChar.m_c;

  // Sign?
  if (c == '-') {
    digits.push_back((char)c);

    // Prepare to consume digits.
    c = readChar();
    if (!isASCIIDigit(c)) {
      unexpectedCharErr(c,
        "looking for digit after minus sign that starts a number");
    }
  }

  // The caller assures us that `firstChar` is a hypen or a digit.
  xassert(isASCIIDigit(c));
  int const firstDigit = c;

  // Next.
  digits.push_back((char)c);
  c = readChar();
  if (c == eofCode()) {
    putback(c);
  }
  else {
    // Radix?
    int radix = 0;
    bool hasRadixIndicator = false;
    if (firstDigit == '0') {
      radix = decodeRadixIndicatorLetter(c);
    }
    if (radix) {
      hasRadixIndicator = true;

      // Store the indicator.
      digits.push_back((char)c);

      // Get the character after the radix indicator.
      c = readNotEOFCharOrErr(
        "looking for digit after radix indicator in integer");
    }
    else {
      radix = 10;
    }

    // Digits after the first.
    while (isASCIIRadixDigit(c, radix)) {
      // Next.
      digits.push_back((char)c);
      c = readChar();
    }

    // Float indicator?
    if (c == '.' || c == 'e' || c == 'E') {
      if (hasRadixIndicator) {
        // The format does not currently allow for (e.g.) hex floats.
        unexpectedCharErr(c,
          "looking for digit in integer after a radix indicator");
      }
      else {
        return continueReadingFloat(firstChar.m_loc, digits, c);
      }
    }

    putbackAfterValueOrErr(c);
  }

  try {
    // This will re-do the radix detection.  That is fine.
    return GDValue(
      GDVInteger::fromDigits(
        std::string_view(digits.data(), digits.size())),
      firstChar.m_loc);
  }
  catch (XFormat &x) {       // gcov-ignore
    // We already validated the syntax, so this should not be possible.
    // But if it happens, map it into an `XReader` for uniformity.
    err(x.getMessage());     // gcov-ignore
    return GDValue();        // Not reached.
  }
}


GDValue GDValueReader::continueReadingFloat(
  GDValueSourceLocation firstLoc,
  std::vector<char> &digits,
  int c)
{
  // This is useful to test how layers above this one react to
  // syntax errors.  Floats are fairly rare, so I have decent
  // control over exactly where it happens.  They are also typically
  // parsed somewhat deep in the data, so recovering from down here
  // is a good exercise.
  static bool injectError =
    envAsBool("GDVALUE_READER_INJECT_FLOAT_ERROR");
  if (injectError) {
    unexpectedCharErr(c,
      "<injected syntax error due to GDVALUE_READER_INJECT_FLOAT_ERROR>");
  }

  xassert(c == '.' || c == 'e' || c == 'E');

  // Fractional part?
  if (c == '.') {
    digits.push_back(c);
    c = readChar();

    // We require at least one digit after the decimal point.
    if (!isASCIIRadixDigit(c, 10)) {
      unexpectedCharErr(c,
        "looking for digit after '.' in float");
    }

    // Accumulate digits.
    while (isASCIIRadixDigit(c, 10)) {
      digits.push_back((char)c);
      c = readChar();
    }
  }

  // Exponent?
  if (c == 'e' || c == 'E') {
    digits.push_back((char)c);
    c = readChar();

    if (c == '-' || c == '+') {
      digits.push_back((char)c);
      c = readChar();

      // A digit must follow the sign.
      if (!isASCIIRadixDigit(c, 10)) {
        unexpectedCharErr(c,
          "looking for digit after sign after exponent indicator in float");
      }
    }
    else {
      // A digit or sign must follow the exponent indicator, and we have
      // ruled out a sign.
      if (!isASCIIRadixDigit(c, 10)) {
        unexpectedCharErr(c,
          "looking for digit or sign after exponent indicator in float");
      }
    }

    // Accumulate digits.
    while (isASCIIRadixDigit(c, 10)) {
      digits.push_back((char)c);
      c = readChar();
    }
  }

  putbackAfterValueOrErr(c);

  try {
    return GDValue(
      GDVBinary64Float::parseString(
        std::string_view(digits.data(), digits.size())),
      firstLoc);
  }
  catch (XFormat &x) {
    // This could happen due to the value being out of range, for
    // example, "1e400".
    err(x.getMessage());
    return GDValue();        // Not reached.
  }
}


GDValue GDValueReader::readNextSymbolOrTaggedContainer(
  GDValueSourceLocationAndChar const &firstChar)
{
  std::string symName;
  if (firstChar.m_c == '`') {
    symName = readNextQuotedStringContents(firstChar.m_c);
  }
  else {
    // Read an unquoted symbol name.

    // We will accumulate the letters of the symbol here.
    std::vector<char> letters;
    letters.push_back(static_cast<char>(firstChar.m_c));

    int c;
    while (true) {
      c = readChar();
      if (!isCIdentifierCharacter(c)) {
        putback(c);
        break;
      }

      letters.push_back(static_cast<char>(c));
    }

    symName = std::string(letters.begin(), letters.end());
  }
  GDVSymbol symbol(symName);

  // Check if the next character is the opening delimiter of a
  // container.
  GDValueSourceLocationAndChar containerOpeningDelim = readLocChar();
  if (containerOpeningDelim.m_c == '{') {
    // Tagged set or map.  First parse the container by itself.
    GDValue container = readNextPossibleMap(containerOpeningDelim);
    if (container.isSet()) {
      // Move the set into a tagged set object.
      return GDValue(
        GDVTaggedSet(symbol,
          std::move(container.setGetMutable())),
        firstChar.m_loc);
    }
    else {
      // Make a tagged map.
      return GDValue(
        GDVTaggedMap(symbol,
          std::move(container.mapGetMutable())),
        firstChar.m_loc);
    }
  }

  else if (containerOpeningDelim.m_c == '[') {
    // Tagged sequence or ordered map.
    GDValue container = readNextPossibleMap(containerOpeningDelim);
    if (container.isOrderedMap()) {
      return GDValue(
        GDVTaggedOrderedMap(symbol,
          std::move(container.orderedMapGetMutable())),
        firstChar.m_loc);
    }
    else {
      return GDValue(
        GDVTaggedSequence(symbol,
          std::move(container.sequenceGetMutable())),
        firstChar.m_loc);
    }
  }

  else if (containerOpeningDelim.m_c == '(') {
    // Tagged tuple.
    GDValue containedTuple = readNextTuple(containerOpeningDelim);
    return GDValue(
      GDVTaggedTuple(symbol,
        std::move(containedTuple.tupleGetMutable())),
      firstChar.m_loc);
  }

  else {
    // Just a symbol.
    putbackAfterValueOrErr(containerOpeningDelim.m_c);  // Could be EOF, fine.
    return GDValue(symbol, firstChar.m_loc);
  }
}


FileLineCol const &GDValueReader::getLocation() const
{
  return m_location;
}


void GDValueReader::skipWhitespaceAndComments()
{
  putback(readCharAfterWhitespaceAndComments());
}


std::optional<GDValue> GDValueReader::readNextValue()
{
  // TODO: This just reads one byte at a time, whereas my spec says
  // it is UTF-8.

  while (true) {
    GDValueSourceLocationAndChar locChar =
      readLocCharAfterWhitespaceAndComments();
    if (locChar.m_c == eofCode()) {
      // Restore 'm_location' to that of the EOF.
      putback(locChar.m_c);
      return std::nullopt;
    }

    switch (locChar.m_c) {
      case ']':
      case '}':
      case ')':
        putback(locChar.m_c);
        return std::nullopt;

      case '[':
        return std::make_optional(readNextPossibleMap(locChar));

      case '{':
        return std::make_optional(readNextPossibleMap(locChar));

      case '(':
        return std::make_optional(readNextTuple(locChar));

      case '"':
        return std::make_optional(readNextDQString(locChar));

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '-':
        return std::make_optional(readNextNumber(locChar));

      default:
        if (isLetter(locChar.m_c) ||
            locChar.m_c == '_' ||
            locChar.m_c == '`') {
          return std::make_optional(
            readNextSymbolOrTaggedContainer(locChar));
        }
        else {
          unexpectedLocCharErr(locChar,
            "looking for the start of a value");
        }
        return std::nullopt;      // Not reached.
    }

    // Not reached.
  }
}


GDValue GDValueReader::readExactlyOneValue()
{
  std::optional<GDValue> ret(readNextValue());
  if (!ret) {
    // Either EOF or a closing delimiter.  We need to re-read the
    // character to determine which.
    unexpectedCharErr(readChar(), "looking for the start of a value");
  }

  // Consume text after the value.
  readEOFOrErr();

  return std::move(*ret);
}


CLOSE_NAMESPACE(gdv)


// EOF
