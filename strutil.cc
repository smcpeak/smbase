// strutil.cc            see license.txt for copyright and terms of use
// code for strutil.h

#include "strutil.h"                   // this module

// smbase
#include "array.h"                     // Array
#include "autofile.h"                  // AutoFILE
#include "c-string-reader.h"           // decodeCStringEscapesToString, parseQuotedCString
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


void decodeEscapes(ArrayStack<char> &dest, std::string const &src,
                   char delim, bool allowNewlines)
{
  std::string decoded =
    decodeCStringEscapesToString(src, delim, allowNewlines);
  for (char c : decoded) {
    dest.push(c);
  }
}


std::string parseQuotedString(std::string const &text)
{
  return parseQuotedCString(text);
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
