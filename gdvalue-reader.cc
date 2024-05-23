// gdvalue-reader.cc
// Code for gdvalue-reader.h.

#include "gdvalue-reader.h"            // this module

#include "breaker.h"                   // breaker
#include "codepoint.h"                 // isWhitespace
#include "gdvalue-reader-exception.h"  // GDValueReaderException
#include "gdvsymbol.h"                 // GDVSymbol
#include "overflow.h"                  // addWithOverflowCheck, multiplyWithOverflowCheck
#include "string-utils.h"              // possiblyTruncatedWithEllipsis

#include <iomanip>                     // std::hex
#include <utility>                     // std::move


namespace gdv {


GDValueReader::GDValueReader(std::istream &is,
                             std::optional<std::string> fileName)
  : m_is(is),
    m_location(fileName)
{}


GDValueReader::~GDValueReader()
{}


/*static*/ constexpr int GDValueReader::eofCode()
{
  return std::istream::traits_type::eof();
}


void GDValueReader::err(string const &syntaxError) const
{
  errAt(m_location, syntaxError);
}


void GDValueReader::errAt(FileLineCol const &loc,
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


void GDValueReader::errUnexpectedChar(int c, char const *lookingFor) const
{
  errUnexpectedCharInCtx(c, stringbc("while " << lookingFor));
}


void GDValueReader::errUnexpectedCharInCtx(int c, char const *context) const
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


void GDValueReader::readExpectChar(int expectChar, char const *lookingFor)
{
  processExpectChar(readChar(), expectChar, lookingFor);
}


void GDValueReader::processExpectChar(int actualChar, int expectChar,
                                      char const *lookingFor)
{
  if (actualChar != expectChar) {
    errUnexpectedChar(actualChar, lookingFor);
  }
}


int GDValueReader::readCharNotEOF(char const *lookingFor)
{
  int c = readChar();
  if (c == eofCode()) {
    errUnexpectedChar(c, lookingFor);
  }
  return c;
}


void GDValueReader::readExpectEOF()
{
  int c = skipWhitespaceAndComments();
  if (c != eofCode()) {
    errUnexpectedChar(c, "looking for the end of a file that should only have one value");
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


void GDValueReader::checkAllowedAfterValue(int c)
{
  if (!isAllowedAfterValue(c)) {
    errUnexpectedCharInCtx(c,
      "after a value; every value must be followed by EOF, whitespace, "
      "',', ':', ']', or '}'");
  }
}


void GDValueReader::putbackAfterValue(int c)
{
  checkAllowedAfterValue(c);
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
          errUnexpectedChar(c, "looking for character after '/'");
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
  auto readCommentCharNoEOF = [&]() -> int {
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

      errUnexpectedChar(c, oss.str().c_str());
    }
    return c;
  };

  while (true) {
    int c = readCommentCharNoEOF();
    switch (c) {
      case '/':
        c = readCommentCharNoEOF();
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
        c = readCommentCharNoEOF();
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
      readExpectChar(']', "looking for ']' at end of sequence");
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
      readExpectChar('}', "looking for \"}}\" at end of set");
      readExpectChar('}', "looking for '}' immediately after '}' at end of set");
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
      readExpectChar('}', "looking for '}' at end of map");
      return ret;
    }

    int colon = skipWhitespaceAndComments();

    processExpectChar(colon, ':', "looking for ':' in map entry");

    // Read the value.
    std::optional<GDValue> value = readNextValue();
    if (!value) {
      errUnexpectedChar(readChar(), "looking for value after ':' in map entry");
    }

    if (ret.mapContains(*key)) {
      // Get the key as GDVN.
      std::string keyAsString =
        possiblyTruncatedWithEllipsis(
          key->asString(), 60);

      // Use the location we saved before.
      FileLineCol loc(m_location);
      loc.setLineCol(keyLC);

      errAt(loc, stringb("Duplicate map key: " << keyAsString));
    }

    ret.mapSetValueAt(std::move(*key), std::move(*value));
  }
}


GDValue GDValueReader::readNextDQString()
{
  std::vector<char> strChars;

  while (true) {
    int c = readCharNotEOF("looking for closing '\"' in double-quoted string");

    if (c == '"') {
      return GDValue(std::string(strChars.begin(), strChars.end()));
    }

    if (c == '\\') {
      c = readCharNotEOF("looking for character after '\\' in double-quoted string");

      // Interpret what follows the backslash.
      //
      // TODO: This is incomplete.  For the moment I am just doing
      // enough to invert what 'doubleQuote' does.  But I need to decide
      // exactly what my encoding scheme is first.
      switch (c) {
        // Characters that denote themselves.
        case '\\':
        case '"':
        case '\'':
          strChars.push_back(c);
          break;

        case 'a':
          strChars.push_back('\a');
          break;

        case 'b':
          strChars.push_back('\b');
          break;

        case 'f':
          strChars.push_back('\f');
          break;

        case 'n':
          strChars.push_back('\n');
          break;

        case 'r':
          strChars.push_back('\r');
          break;

        case 't':
          strChars.push_back('\t');
          break;

        case 'v':
          strChars.push_back('\v');
          break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
          // Decode octal.
          int decoded = c - '0';
          while (true) {
            c = readCharNotEOF("looking for digits in octal escape sequence in double-quoted string");
            if (isASCIIOctDigit(c)) {
              try {
                decoded = addWithOverflowCheck(
                            multiplyWithOverflowCheck(decoded, 8),
                            c - '0');
              }
              catch (XOverflow const &) {
                err(stringb("Code point denoted by octal escape in double-quoted string is too large."));
              }
            }
            else {
              // Put the character after the escape sequence back.
              putback(c);

              // Store the decoded value.
              //
              // TODO: This discards values larger than 127-ish.
              strChars.push_back((char)decoded);
              break;
            }
          }
          break;
        }

        default:
          errUnexpectedChar(c, "looking for the character after a '\\' in a double-quoted string");
          break;
      } // switch(c) after backslash
    } // if(backslash)

    else /* not backslash, double-quote, or EOF */ {
      // Ordinary character.
      strChars.push_back((char)c);
    }
  } // while(true)

  // Not reached.
}


GDValue GDValueReader::readNextInteger(int firstChar)
{
  bool isNegative = false;
  if (firstChar == '-') {
    isNegative = true;

    // Prepare to consume digits.
    firstChar = readCharNotEOF(
      "looing for digit after minus sign that starts an integer");
  }

  GDVInteger integerValue(firstChar - '0');

  try {
    while (true) {
      int c = readChar();
      if (!isASCIIDigit(c)) {
        putbackAfterValue(c);
        break;
      }

      integerValue =
        addWithOverflowCheck(
          multiplyWithOverflowCheck(integerValue, (GDVInteger)10),
          (GDVInteger)(c - '0'));
    }

    if (isNegative) {
      // This can't overflow since the negative magnitude range is
      // larger than the positive, but I'm being defensive.
      integerValue =
        multiplyWithOverflowCheck(integerValue, (GDVInteger)-1);
    }

    return GDValue(integerValue);
  }

  catch (XOverflow const &) {
    // I want to eventually use an arbitrary-precision integer
    // representation, in which case this error will be impossible.  But
    // in the meantime I'll just note that this code will misbehave on
    // MIN_INT64 since the accumulation of the positive value will
    // overflow even though it would fit if negative.
    err(stringb("Value denoted by integer is too large."));
    return GDValue();        // Not reached.
  }
}


GDValue GDValueReader::readNextSymbolOrSpecial(int firstChar)
{
  std::vector<char> letters;
  letters.push_back((char)firstChar);

  while (true) {
    int c = readChar();
    if (!isCIdentifierCharacter(c)) {
      putbackAfterValue(c);       // Could be EOF, fine.
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
          errUnexpectedCharInCtx(c, "after '{'");
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
          errUnexpectedChar(c, "looking for the start of a value");
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
    errUnexpectedChar(readChar(), "looking for the start of a value");
  }

  // Consume text after the value.
  readExpectEOF();

  return std::move(*ret);
}


} // namespace gdv


// EOF
