// gdvalue-reader.cc
// Code for gdvalue-reader.h.

// This file is in the public domain.

#include "gdvalue-reader.h"            // this module

#include "breaker.h"                   // breaker
#include "codepoint.h"                 // isWhitespace, decodeRadixIndicatorLetter, isASCIIRadixDigit
#include "gdvalue-reader-exception.h"  // GDValueReaderException
#include "gdvsymbol.h"                 // GDVSymbol
#include "overflow.h"                  // addWithOverflowCheck, multiplyWithOverflowCheck
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "string-utils.h"              // possiblyTruncatedWithEllipsis
#include "utf8-writer.h"               // smbase::UTF8Writer

#include <iomanip>                     // std::hex
#include <utility>                     // std::move

using namespace smbase;


OPEN_NAMESPACE(gdv)


GDValueReader::GDValueReader(std::istream &is,
                             std::optional<std::string> fileName)
  : m_is(is),
    m_location(fileName)
{}


GDValueReader::~GDValueReader()
{}


STATICDEF constexpr int GDValueReader::eofCode()
{
  return std::istream::traits_type::eof();
}


void GDValueReader::err(string const &syntaxError) const
{
  locErr(m_location, syntaxError);
}


void GDValueReader::locErr(FileLineCol const &loc,
                           string const &syntaxError) const
{
  // Generally, we read a character, advancing the location in the
  // process, then check for an error.  Consequently, when we report an
  // error the location is one past the place the erroneous character
  // was.  So, back the location spot up.
  FileLineCol prev(loc);
  prev.decrementColumn();

  breaker();
  throw GDValueReaderException(prev, syntaxError);
}


void GDValueReader::unexpectedCharErr(int c, char const *lookingFor) const
{
  inCtxUnexpectedCharErr(c, stringbc("while " << lookingFor));
}


void GDValueReader::inCtxUnexpectedCharErr(int c, char const *context) const
{
  if (c == eofCode()) {
    err(stringb("Unexpected end of file " <<
                context << "."));
  }
  else if (isASCIIPrintable(c)) {
    err(stringb("Unexpected '" << (char)c << "' " <<
                context << "."));
  }
  else {
    err(stringb("Unexpected unprintable character code " <<
                (unsigned)c << " (0x" << std::hex << std::setw(2) <<
                std::setfill('0') << (unsigned)c <<
                ") " << context << "."));
  }

  // Not reached.
}


int GDValueReader::readChar()
{
  int c = m_is.get();

  // Update the location.
  //
  // Note: We do this even for EOF for uniformity.
  m_location.incrementForChar(c);

  return c;
}


void GDValueReader::readCharOrErr(int expectChar, char const *lookingFor)
{
  processCharOrErr(readChar(), expectChar, lookingFor);
}


void GDValueReader::processCharOrErr(int actualChar, int expectChar,
                                     char const *lookingFor)
{
  if (actualChar != expectChar) {
    unexpectedCharErr(actualChar, lookingFor);
  }
}


int GDValueReader::readNotEOFCharOrErr(char const *lookingFor)
{
  int c = readChar();
  if (c == eofCode()) {
    unexpectedCharErr(c, lookingFor);
  }
  return c;
}


void GDValueReader::readEOFOrErr()
{
  int c = skipWhitespaceAndComments();
  if (c != eofCode()) {
    unexpectedCharErr(c, "looking for the end of a file that should only have one value");
  }
}


void GDValueReader::putback(int c)
{
  if (c == eofCode()) {
    // It is convenient to allow this to make the parsing code more
    // uniform in its treatment of EOF versus other terminators in
    // some places.  But we do not actually put anything back into the
    // stream.
  }
  else {
    m_is.putback(c);
  }

  // Either way, however, the location must be decremented because it
  // was incremented when when we did the corresponding 'readChar', even
  // if it returned EOF.
  m_location.decrementForChar(c);
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
      "',', ':', ']', or '}'");
  }
}


void GDValueReader::putbackAfterValueOrErr(int c)
{
  checkAfterValueOrErr(c);
  putback(c);
}


int GDValueReader::skipWhitespaceAndComments()
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


GDValue GDValueReader::readNextSequence()
{
  GDValue ret(GDVK_SEQUENCE);

  while (true) {
    std::optional<GDValue> next = readNextValue();
    if (!next) {
      readCharOrErr(']', "looking for ']' at end of sequence");
      return ret;
    }

    ret.sequenceAppend(std::move(*next));
  }
}


GDValue GDValueReader::readNextSet()
{
  GDValue ret(GDVK_SET);

  while (true) {
    std::optional<GDValue> next = readNextValue();
    if (!next) {
      readCharOrErr('}', "looking for \"}}\" at end of set");
      readCharOrErr('}', "looking for '}' immediately after '}' at end of set");
      return ret;
    }

    ret.setInsert(std::move(*next));
  }
}


GDValue GDValueReader::readNextMap()
{
  GDValue ret(GDVK_MAP);

  while (true) {
    // Skip leading whitespace.
    int firstKeyChar = skipWhitespaceAndComments();

    // Save this location as the key location in case we need to report
    // a duplicate key error below.
    LineCol keyLC = m_location.getLineCol();

    // Put the first key character back so 'readNextValue' will see it.
    putback(firstKeyChar);

    // Read the key.
    std::optional<GDValue> key = readNextValue();
    if (!key) {
      readCharOrErr('}', "looking for '}' at end of map");
      return ret;
    }

    int colon = skipWhitespaceAndComments();

    processCharOrErr(colon, ':', "looking for ':' in map entry");

    // Read the value.
    std::optional<GDValue> value = readNextValue();
    if (!value) {
      unexpectedCharErr(readChar(), "looking for value after ':' in map entry");
    }

    if (ret.mapContains(*key)) {
      // Get the key as GDVN.
      std::string keyAsString =
        possiblyTruncatedWithEllipsis(
          key->asString(), 60);

      // Use the location we saved before.
      FileLineCol loc(m_location);
      loc.setLineCol(keyLC);

      locErr(loc, stringb("Duplicate map key: " << keyAsString));
    }

    ret.mapSetValueAt(std::move(*key), std::move(*value));
  }
}


GDValue GDValueReader::readNextDQString()
{
  std::ostringstream oss;
  UTF8Writer utf8Writer(oss);

  while (true) {
    int c = readNotEOFCharOrErr("looking for closing '\"' in double-quoted string");

    if (c == '"') {
      break;
    }

    if (c == '\\') {
      c = readNotEOFCharOrErr("looking for character after '\\' in double-quoted string");

      // Interpret what follows the backslash.
      switch (c) {
        // Characters that denote themselves.
        case '"':
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

        case 'u': {
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
                decoded = decodeSurrogatePair(decoded, decoded2);
              }
              else {
                err(stringf(
                  "Expected low surrogate in [U+DC00,U+DFFF], "
                  "but instead found \"\\u%04X\".",
                  decoded2));
              }
            }
            catch (GDValueReaderException &e) {
              // We do not compute the more detailed context above
              // because we do not yet know we will report an error.
              e.prependGDVNContext(stringf(
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

          // Store the decoded value as UTF-8.
          utf8Writer.writeCodePoint(decoded);
          break;
        }

        default:
          unexpectedCharErr(c, "looking for the character after a '\\' in a double-quoted string");
          break;
      } // switch(c) after backslash
    } // if(backslash)

    else /* not backslash, double-quote, or EOF */ {
      // Ordinary character.
      oss << (char)c;
    }
  } // while(true)

  return GDValue(oss.str());
}


int GDValueReader::readNextU4Escape()
{
  int decoded = 0;
  for (int i=0; i < 4; ++i) {
    int c = readChar();
    if (isASCIIHexDigit(c)) {
      decoded = decoded*16 + decodeASCIIHexDigit(c);
    }
    else {
      unexpectedCharErr(c, "looking for digits in \"\\u\" escape sequence in double-quoted string");
    }
  }

  return decoded;
}


GDValue GDValueReader::readNextInteger(int const firstChar)
{
  // We will collect all of the characters of the number here before
  // interpreting them as a number.
  std::vector<char> digits;

  // In the steady state, `c` has the next character to process.
  int c = firstChar;

  // Sign?
  if (c == '-') {
    digits.push_back((char)c);

    // Prepare to consume digits.
    c = readChar();
    if (!isASCIIDigit(c)) {
      unexpectedCharErr(c,
        "looking for digit after minus sign that starts an integer");
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
    if (firstDigit == '0') {
      radix = decodeRadixIndicatorLetter(c);
    }
    if (radix) {
      // Next.
      digits.push_back((char)c);
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

    putbackAfterValueOrErr(c);
  }

  try {
    // This will re-do the radix detection.  That is fine.
    return GDValue(GDVInteger::fromRadixPrefixedDigits(
      std::string_view(digits.data(), digits.size())));
  }
  catch (XFormat &x) {
    // We already validated the syntax, so this should not be possible.
    // But if it happens, map it into a `GDValueReaderException` for
    // uniformity.
    err(x.getMessage());
    return GDValue();  // Not reached.
  }
}


GDValue GDValueReader::readNextSymbolOrSpecial(int firstChar)
{
  std::vector<char> letters;
  letters.push_back((char)firstChar);

  while (true) {
    int c = readChar();
    if (!isCIdentifierCharacter(c)) {
      putbackAfterValueOrErr(c);       // Could be EOF, fine.
      break;
    }

    letters.push_back((char)c);
  }

  std::string symName(letters.begin(), letters.end());

  if (symName == "null") {
    return GDValue();
  }
  else {
    return GDValue(GDVSymbol(symName));
  }
}


std::optional<GDValue> GDValueReader::readNextValue()
{
  // TODO: This just reads one byte at a time, whereas my spec says
  // it is UTF-8.

  while (true) {
    int c = skipWhitespaceAndComments();
    if (c == eofCode()) {
      // Restore 'm_location' to that of the EOF.
      putback(c);
      return std::nullopt;
    }

    switch (c) {
      case ']':
      case '}':
        putback(c);
        return std::nullopt;

      case '[':
        return std::make_optional(readNextSequence());

      case '{': {
        c = readChar();
        if (c == eofCode()) {
          inCtxUnexpectedCharErr(c, "after '{'");
          return std::nullopt;    // Not reached.
        }
        else if (c == '{') {
          return std::make_optional(readNextSet());
        }
        else {
          putback(c);
          return std::make_optional(readNextMap());
        }
      }

      case '"':
        return std::make_optional(readNextDQString());

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
        return std::make_optional(readNextInteger(c));

      default:
        if (isLetter(c) || c == '_') {
          return std::make_optional(readNextSymbolOrSpecial(c));
        }
        else {
          unexpectedCharErr(c, "looking for the start of a value");
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
