// strutil.cc            see license.txt for copyright and terms of use
// code for strutil.h

#include "strutil.h"                   // this module

// smbase
#include "array.h"                     // Array
#include "autofile.h"                  // AutoFILE
#include "codepoint.h"                 // isPrintableASCII, isShellMetacharacter
#include "compare-util.h"              // compare
#include "exc.h"                       // smbase::xformat
#include "nonport.h"                   // vnprintf

// libc
#include <ctype.h>                     // isspace
#include <stdio.h>                     // sprintf
#include <stdlib.h>                    // strtoul, qsort
#include <string.h>                    // strstr, memcmp
#include <time.h>                      // time, asctime, localtime
#include <unistd.h>                    // write

using namespace smbase;


// replace all instances of oldstr in src with newstr, return result
string replace(rostring origSrc, rostring oldstr, rostring newstr)
{
  stringBuilder ret;
  char const *src = toCStr(origSrc);

  while (*src) {
    char const *next = strstr(src, toCStr(oldstr));
    if (!next) {
      ret << src;
      break;
    }

    // add the characters between src and next
    ret << substring(src, next-src);

    // add the replace-with string
    ret << newstr;

    // move src to beyond replaced substring
    src += (next-src) + strlen(oldstr);
  }

  return ret.str();
}


string expandRanges(char const *chars_)
{
  stringBuilder ret;

  // Fix from Hendrik Tews: use unsigned chars to as not to fall over
  // when ranges have values near 127 on compilers for which 'char' is
  // signed by default (which is probably the common case)
  unsigned char *chars = (unsigned char*)chars_;

  while (*chars) {
    if (chars[1] == '-' && chars[2] != 0) {
      // range specification
      if (chars[0] > chars[2]) {
        xformat("range specification with wrong collation order");
      }

      // use 'int' so we can handle chars[2] == 255 (otherwise we get
      // infinite loop)
      for (int c = chars[0]; c <= chars[2]; c++) {
        ret << (unsigned char)c;
      }
      chars += 3;
    }
    else {
      // simple character specification
      ret << chars[0];
      chars++;
    }
  }

  return ret.str();
}


string translate(rostring origSrc, rostring srcchars, rostring destchars)
{
  // first, expand range notation in the specification sequences
  string srcSpec = expandRanges(toCStr(srcchars));
  string destSpec = expandRanges(toCStr(destchars));

  // build a translation map
  char map[256];
  string::size_type i;
  for (i=0; i<256; i++) {
    map[i] = i;
  }

  // excess characters from either string are ignored ("SysV" behavior)
  for (i=0; i < srcSpec.length() && i < destSpec.length(); i++) {
    map[(unsigned char)( srcSpec[i] )] = destSpec[i];
  }

  // run through 'src', applying 'map'
  char const *src = toCStr(origSrc);
  Array<char> ret(strlen(src)+1);
  char *dest = ret.ptr();
  while (*src) {
    *dest = map[(unsigned char)*src];
    dest++;
    src++;
  }
  *dest = 0;    // final nul terminator

  return string(ret.ptrC());
}


string stringToupper(rostring src)
{
  return translate(src, "a-z", "A-Z");
}

string stringTolower(rostring src)
{
  return translate(src, "A-Z", "a-z");
}


string trimWhitespace(rostring origStr)
{
  char const *str = toCStr(origStr);

  // trim leading whitespace
  while (isspace(*str)) {
    str++;
  }

  // trim trailing whitespace
  char const *end = str + strlen(str);
  while (end > str &&
         isspace(end[-1])) {
    end--;
  }

  // return it
  return substring(str, end-str);
}


string firstAlphanumToken(rostring origStr)
{
  char const *str = toCStr(origStr);

  // find the first alpha-numeric; NOTE: if we hit the NUL at the end,
  // that should not be alpha-numeric and we should stop
  while(!isalnum(*str)) {
    str++;
  }

  // keep going until we are not at an alpha-numeric
  char const *end = str;
  while(isalnum(*end)) {
    end++;
  }

  // return it
  return substring(str, end-str);
}


// table of escape codes
static struct Escape {
  char actual;      // actual character in string
  char escape;      // char that follows backslash to produce 'actual'
} const escapes[] = {
  { '\0', '0' },  // nul
  { '\a', 'a' },  // bell
  { '\b', 'b' },  // backspace
  { '\f', 'f' },  // form feed
  { '\n', 'n' },  // newline
  { '\r', 'r' },  // carriage return
  { '\t', 't' },  // tab
  { '\v', 'v' },  // vertical tab
  { '\\', '\\'},  // backslash
  { '"',  '"' },  // double-quote
  { '\'', '\''},  // single-quote
};


string encodeWithEscapes(char const *p, int len)
{
  stringBuilder sb;

  for (; len>0; len--, p++) {
    // look for an escape code
    unsigned i;
    for (i=0; i<TABLESIZE(escapes); i++) {
      if (escapes[i].actual == *p) {
        sb << '\\' << escapes[i].escape;
        break;
      }
    }
    if (i<TABLESIZE(escapes)) {
      continue;   // found it and printed it
    }

    // try itself
    if (isprint(*p)) {
      sb << *p;
      continue;
    }

    // use the most general notation
    char tmp[5];
    sprintf(tmp, "\\x%02X", (unsigned char)(*p));
    sb << tmp;
  }

  return sb.str();
}


string encodeWithEscapes(rostring p)
{
  return encodeWithEscapes(toCStr(p), strlen(p));
}


string quoted(rostring src)
{
  return stringb("\"" << encodeWithEscapes(src) << "\"");
}


void decodeEscapes(ArrayStack<char> &dest, rostring origSrc,
                   char delim, bool allowNewlines)
{
  char const *src = toCStr(origSrc);

  while (*src != '\0') {
    if (*src == '\n' && !allowNewlines) {
      xformat("unescaped newline (unterminated string)");
    }
    if (*src == delim) {
      xformat(stringb("unescaped delimiter (" << delim << ")"));
    }

    if (*src != '\\') {
      // easy case
      dest.push(*src);
      src++;
      continue;
    }

    // advance past backslash
    src++;

    // see if it's a simple one-char backslash code;
    // start at 1 so we do *not* use the '\0' code since
    // that's actually a special case of \0123', and
    // interferes with the latter
    int i;
    for (i=1; i<TABLESIZE(escapes); i++) {
      if (escapes[i].escape == *src) {
        dest.push(escapes[i].actual);
        src++;
        break;
      }
    }
    if (i < TABLESIZE(escapes)) {
      continue;
    }

    if (*src == '\0') {
      xformat("backslash at end of string");
    }

    if (*src == '\n') {
      // escaped newline; advance to first non-whitespace
      src++;
      while (*src==' ' || *src=='\t') {
        src++;
      }
      continue;
    }

    if (*src == 'x' || isdigit(*src)) {
      // hexadecimal or octal char (it's unclear to me exactly how to
      // parse these since it's supposedly legal to have e.g. "\x1234"
      // mean a one-char string.. whatever)
      bool hex = (*src == 'x');
      if (hex) {
        src++;

        // strtoul is willing to skip leading whitespace, so I need
        // to catch it myself
        if (isspace(*src)) {
          xformat("whitespace following hex (\\x) escape");
        }
      }

      char const *endptr;
      unsigned long val = strtoul(src, (char**)&endptr, hex? 16 : 8);
      if (src == endptr) {
        // this can't happen with the octal escapes because
        // there is always at least one valid digit
        xformat("invalid hex (\\x) escape");
      }

      dest.push((char)(unsigned char)val);    // possible truncation..
      src = endptr;
      continue;
    }

    // everything not explicitly covered will be considered
    // an error (for now), even though the C++ spec says
    // it should be treated as if the backslash were not there
    //
    // 7/29/04: now making backslash the identity transformation in
    // this case
    //
    // copy character as if it had not been backslashed
    dest.push(*src);
    src++;
  }
}


string parseQuotedString(rostring text)
{
  if (!( text[0] == '"' &&
         text[strlen(text)-1] == '"' )) {
    xformat(stringb("quoted string is missing quotes: " << text));
  }

  // strip the quotes
  string noQuotes = substring(toCStr(text)+1, strlen(text)-2);

  // decode escapes
  ArrayStack<char> buf;
  decodeEscapes(buf, noQuotes, '"');
  buf.push(0 /*NUL*/);

  // return string contents up to first NUL, which isn't necessarily
  // the same as the one just pushed; by invoking this function, the
  // caller is accepting responsibility for this condition
  return string(buf.getArray());
}


string quoteCharacter(int c)
{
  if (isASCIIPrintable(c)) {
    if (c == '\'') {
      return "'\\''";
    }
    else if (c == '\\') {
      return "'\\\\'";
    }
    else {
      return stringb('\'' << (char)c << '\'');
    }
  }

  // Treat a negative as a large positive.
  unsigned uc = (unsigned)c;

  if (uc <= 0xFF) {
    return stringf("\\x%02X", uc);
  }
  else if (uc <= 0xFFFF) {
    return stringf("\\u%04X", uc);
  }
  else {
    return stringf("\\U%08X", uc);
  }
}


static bool hasShellMetaOrNonprint(string const &s)
{
  int len = s.length();
  for (int i=0; i<len; i++) {
    int c = (unsigned char)s[i];
    if (!isASCIIPrintable(c)) {
      return true;
    }
    if (isShellMetacharacter(c)) {
      return true;
    }
  }
  return false;
}

// Reference on shell double-quote syntax in the POSIX shell:
// http://pubs.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_02_03
string shellDoubleQuote(string const &s)
{
  if (s.empty() || hasShellMetaOrNonprint(s)) {
    stringBuilder sb;
    sb << '"';

    int len = s.length();
    for (int i=0; i<len; i++) {
      char c = s[i];
      switch (c) {
        // Within a double-quoted string, only these four characters
        // need to or can be escaped.
        case '$':
        case '`':
        case '"':
        case '\\':
          sb << '\\' << c;
          break;

        default:
          sb << c;
          break;
      }
    }

    sb << '"';
    return sb.str();
  }
  else {
    return s;
  }
}


string sm_basename(rostring origSrc)
{
  char const *src = toCStr(origSrc);

  char const *sl = strrchr(src, '/');   // locate last slash
  if (sl && sl[1] == 0) {
    // there is a slash, but it is the last character; ignore it
    // (this behavior is what /bin/basename does)
    return sm_basename(substring(src, strlen(src)-1));
  }

  if (sl) {
    return string(sl+1);     // everything after the slash
  }
  else {
    return string(src);      // entire string if no slashes
  }
}

string dirname(rostring origSrc)
{
  char const *src = toCStr(origSrc);

  char const *sl = strrchr(src, '/');   // locate last slash
  if (sl == src) {
    // last slash is leading slash
    return string("/");
  }

  if (sl && sl[1] == 0) {
    // there is a slash, but it is the last character; ignore it
    // (this behavior is what /bin/dirname does)
    return dirname(substring(src, strlen(src)-1));
  }

  if (sl) {
    return substring(src, sl-src);     // everything before slash
  }
  else {
    return string(".");
  }
}


// I will expand this definition to use more knowledge about English
// irregularities as I need it
string plural(int n, rostring prefix)
{
  if (n==1) {
    return prefix;
  }

  if (0==strcmp(prefix, "was")) {
    return string("were");
  }
  if (prefix[strlen(prefix)-1] == 'y') {
    return stringb(substring(prefix, prefix.length()-1) << "ies");
  }
  else {
    return stringb(prefix << "s");
  }
}

string pluraln(int n, rostring prefix)
{
  return stringb(n << " " << plural(n, prefix));
}


string a_or_an(rostring noun)
{
  bool use_an = false;

  if (strchr("aeiouAEIOU", noun[0])) {
    use_an = true;
  }

  // special case: I pronounce "mvisitor" like "em-visitor"
  if (noun[0]=='m' && noun[1]=='v') {
    use_an = true;
  }

  if (use_an) {
    return stringb("an " << noun);
  }
  else {
    return stringb("a " << noun);
  }
}


char *copyToStaticBuffer(char const *s)
{
  enum { SZ=200 };
  static char buf[SZ+1];

  int len = strlen(s);
  if (len > SZ) len=SZ;
  memcpy(buf, s, len);
  buf[len] = 0;

  return buf;
}


bool prefixEquals(rostring str, rostring prefix)
{
  int slen = strlen(str);
  int plen = strlen(prefix);
  return slen >= plen &&
         0==memcmp(toCStr(str), toCStr(prefix), plen);
}

bool suffixEquals(rostring str, rostring suffix)
{
  int slen = strlen(str);
  int ulen = strlen(suffix);    // sUffix
  return slen >= ulen &&
         0==memcmp(toCStr(str)+slen-ulen, toCStr(suffix), ulen);
}


bool hasSubstring(string const &haystack, string const &needle)
{
  return indexOfSubstring(haystack, needle) >= 0;
}

int indexOfSubstring(string const &haystack, string const &needle)
{
  char const *h = haystack.c_str();
  char const *occ = strstr(h, needle.c_str());
  if (occ) {
    return occ - h;
  }
  else {
    return -1;
  }
}


void writeStringToFile(rostring str, rostring fname)
{
  AutoFILE fp(toCStr(fname), "w");

  if (fputs(toCStr(str), fp) < 0) {
    xmessage("fputs: EOF");
  }
}


string readStringFromFile(rostring fname)
{
  AutoFILE fp(toCStr(fname), "r");

  stringBuilder sb;

  char buf[4096];
  for (;;) {
    int len = fread(buf, 1, 4096, fp);
    if (len < 0) {
      xmessage("fread failed");
    }
    if (len == 0) {
      break;
    }

    sb.append(buf, len);
  }

  return sb.str();
}


void readLinesFromFile(ArrayStack<string> /*INOUT*/ &dest,
                       rostring fname,
                       bool doChomp)
{
  AutoFILE fp(toCStr(fname), "r");
  string line;
  while (readLine(line, fp)) {
    if (doChomp) {
      line = chomp(line);
    }
    dest.push(line);
  }
}


bool readLine(string &dest, FILE *fp)
{
  char buf[80];

  if (!fgets(buf, 80, fp)) {
    return false;
  }

  if (buf[strlen(buf)-1] == '\n') {
    // read a newline, we got the whole line
    dest = buf;
    return true;
  }

  // only got part of the string; need to iteratively construct
  stringBuilder sb;
  sb << buf;
  while (buf[strlen(buf)-1] != '\n') {
    if (!fgets(buf, 80, fp)) {
      // found eof after partial; return partial *without* eof
      // indication, since we did in fact read something
      break;
    }
    sb << buf;
  }

  dest = sb.str();
  return true;
}


string chomp(rostring src)
{
  if (!src.empty() && src[strlen(src)-1] == '\n') {
    return substring(src, strlen(src)-1);
  }
  else {
    return src;
  }
}


DelimStr::DelimStr(char delimiter0)
  : delimiter(delimiter0)
{}

DelimStr& DelimStr::operator<< (char const *text) {
  if (!sb.empty()) sb << delimiter;
  sb << text;
  return *this;
}


int compareStrings(const void *a, const void *b) {
  char const **a1 = (char const **)a;
  char const **b1 = (char const **)b;
  return strcmp(*a1, *b1);
}

void qsortStringArray(char const **strings, int size) {
  qsort(strings, size, sizeof strings[0], compareStrings);
}


int compareStringPtrs(string const *a, string const *b)
{
  return compare(*a, *b);
}


// EOF
