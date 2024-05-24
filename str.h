// str.h            see license.txt for copyright and terms of use
// 2024-05-20: This is a compatibility header.  It declares 'string' as
// an alias for 'std::string', and has some other stuff related to
// legacy usage.  New code should avoid it.

#ifndef SMBASE_STR_H
#define SMBASE_STR_H

#include "flatten-fwd.h" // Flatten
#include "sm-iostream.h" // istream, ostream
#include "stringb.h"     // stringb
#include "stringf.h"     // stringf

#include <string>        // std::string

#include <string.h>      // strcmp, etc.


// ------------------------- string ---------------------
// 2024-05-19: Let's throw the switch and commit to std::string.
using std::string;


// This is used when I want to call a function in smbase::string
// that does not exist or has different semantics in std::string.
// That way for now I can keep using the function, but it is
// marked as incompatible.
enum SmbaseStringFunc { SMBASE_STRING_FUNC };

// This class should only be used in the rare places I really need a
// string with the old semantics.  The vast majority of code should use
// 'string', which is now 'std::string'.
class OldSmbaseString {
public:
  typedef int size_type;
protected:     // data
  // 10/12/00: switching to never letting s be NULL
  char *s;     	       	       	       // string contents; never NULL
  static char * const emptyString;     // a global ""; should never be modified

protected:     // funcs
  void dup(char const *source);        // copies, doesn't dealloc first
  void kill();                         // dealloc if str != 0

public:	       // funcs
  OldSmbaseString(OldSmbaseString const &src) { dup(src.s); }
  OldSmbaseString(char const *src) { dup(src); }
  OldSmbaseString() { s=emptyString; }
  ~OldSmbaseString() { kill(); }

  // for this one, use ::substring instead
  OldSmbaseString(char const *src, int length, SmbaseStringFunc);

  // actually, not sure what I was thinking, std::string has this
  OldSmbaseString(char const *src, int length);

  // for this one, there are two alternatives:
  //   - stringBuilder has nearly the same constructor interface
  //     as string had, but cannot export a char* for writing
  //     (for the same reason string can't anymore); operator[] must
  //     be used
  //   - Array<char> is very flexible, but remember to add 1 to
  //     the length passed to its constructor!
  OldSmbaseString(int length, SmbaseStringFunc)
    { s=emptyString; setlength(length); }

  OldSmbaseString(Flatten&);
  void xfer(Flatten &flat);

  // simple queries
  int length() const;  	       	// returns number of non-null chars in the string; length of "" is 0
  bool isempty() const { return s[0]==0; }
  bool contains(char c) const;

  // std::string has this instead; I will begin using slowly
  bool empty() const { return isempty(); }

  // array-like access
  char& operator[] (int i) { return s[i]; }
  char operator[] (int i) const { return s[i]; }

  // substring
  OldSmbaseString substring(int startIndex, int length) const;

  // conversions
  #if 0    // removing these for more standard compliace
    //operator char* () { return s; }      // ambiguities...
    operator char const* () const { return s; }
    char *pchar() { return s; }
    char const *pcharc() const { return s; }
  #else
    char const *c_str() const { return s; }
  #endif

  // Let's try letting my string implicitly convert to std::string.
  operator std::string () const { return std::string(c_str()); }

  // And vice-versa.
  OldSmbaseString(std::string const &s);

  // assignment
  OldSmbaseString& operator=(OldSmbaseString const &src)
    { if (&src != this) { kill(); dup(src.s); } return *this; }
  OldSmbaseString& operator=(char const *src)
    { if (src != s) { kill(); dup(src); } return *this; }

  // allocate 'newlen' + 1 bytes (for null); initial contents is ""
  OldSmbaseString& setlength(int newlen);

  // comparison; return value has same meaning as strcmp's return value:
  //   <0   if   *this < src
  //   0    if   *this == src
  //   >0   if   *this > src
  int compareTo(OldSmbaseString const &src) const;
  int compareTo(char const *src) const;
  bool equals(char const *src) const { return compareTo(src) == 0; }
  bool equals(OldSmbaseString const &src) const { return compareTo(src) == 0; }

  #define MAKEOP(op)                                                                    \
    bool operator op (OldSmbaseString const &src) const { return compareTo(src) op 0; } \
    bool operator op (const char *src) const { return compareTo(src) op 0; }
    /* killed stuff with char* because compilers are too flaky; use compareTo */
    // 2008-12-13: Re-added char* since the removal of the impicit
    // conversion to char const * makes it unambiguous again.
  MAKEOP(==)  MAKEOP(!=)
  MAKEOP(>=)  MAKEOP(>)
  MAKEOP(<=)  MAKEOP(<)
  #undef MAKEOP

  // 2021-06-10: I used to have operator& and operator&= here, but I
  // have removed them as part of my effort to make this class have an
  // interface compatible with std::string.

  // Concatenation.
  //
  // This is inefficient since repeated concatenation takes quadratic
  // time for N concatenations, that in turn because this 'string' does
  // not separately track its allocated size.  For that, use
  // 'stringBuilder'.
  OldSmbaseString operator+ (OldSmbaseString const &tail) const;
  OldSmbaseString& operator+= (OldSmbaseString const &tail);

  // input/output
  friend istream& operator>> (istream &is, OldSmbaseString &obj)
    { obj.readline(is); return is; }
  friend ostream& operator<< (ostream &os, OldSmbaseString const &obj)
    { obj.write(os); return os; }

  // note: the read* functions are currently implemented in a fairly
  // inefficient manner (one char at a time)

  void readdelim(istream &is, char const *delim);
    // read from is until any character in delim is encountered; consumes that
    // character, but does not put it into the string; if delim is null or
    // empty, reads until EOF

  void readall(istream &is) { readdelim(is, NULL); }
    // read all remaining chars of is into this

  void readline(istream &is) { readdelim(is, "\n"); }
    // read a line from input stream; consumes the \n, but doesn't put it into
    // the string

  void write(ostream &os) const;
    // writes all stored characters (but not '\0')

  // debugging
  void selfCheck() const;
    // fail an assertion if there is a problem
};


// -------------------------- compatibility ----------------------------
// These functions correspond to methods of OldSmbaseString that do not
// exist on std::string.

// Equivalent of OldSmbaseString::xfer(Flatten&) for std::string.
void stringXfer(std::string &str, Flatten &flat);

// Equivalent of OldSmbaseString::equals() for std::string.
bool stringEquals(std::string const &a, char const *b);
bool stringEquals(std::string const &a, std::string const &b);

// Note: The equivalent of OldSmbaseString::substring is just
// std::string::substr, so rather than create 'stringSubstring', I just
// change call sites to use 'substr'.


// ------------------------ rostring ----------------------
// My plan is to use this in places I currently use 'char const *'.
typedef string const &rostring;

// I have the modest hope that the transition to 'rostring' might be
// reversible, so this function converts to 'char const *' but with a
// syntax that could just as easily apply to 'char const *' itself
// (and in that case would be the identity function).
inline char const *toCStr(rostring s) { return s.c_str(); }

// at the moment, if I do this it is a mistake, so catch it; this
// function is not implemented anywhere
void/*unusable*/ toCStr(char const *s);

// I need some compatibility functions
inline size_t strlen(rostring s) { return s.length(); }

// Overload strlen for unsigned char* to avoid annoying casts.
inline size_t strlen(unsigned char const *s) { return strlen((char const*)s); }

// This appears to be unused.
//inline istream &getline(istream &in, OldSmbaseString &line) { line.readline(in); return in; }

int strcmp(rostring s1, rostring s2);
int strcmp(rostring s1, char const *s2);
int strcmp(char const *s1, rostring s2);
// string.h, above, provides:
// int strcmp(char const *s1, char const *s2);

// dsw: this is what we are asking most of the time so let's special
// case it
inline bool streq(rostring s1, rostring s2)       {return strcmp(s1, s2) == 0;}
inline bool streq(rostring s1, char const *s2)    {return strcmp(s1, s2) == 0;}
inline bool streq(char const *s1, rostring s2)    {return strcmp(s1, s2) == 0;}
inline bool streq(char const *s1, char const *s2) {return strcmp(s1, s2) == 0;}

char const *strstr(rostring haystack, char const *needle);

// There is no wrapper for 'strchr'; use the 'contains' function
// declared in string-utils.h.

int atoi(rostring s);

// construct a string out of characters from 'p' up to 'p+n-1',
// inclusive; resulting string length is 'n'
string substring(char const *p, int n);
inline string substring(rostring p, int n)
  { return substring(p.c_str(), n); }


// --------------------- stringBuilder --------------------
// This class is specifically for appending lots of things.
//
// It is one of the few classes that really needs 'OldSmbaseString' to
// work.  New code should use 'std::ostringstream', not this class.
class stringBuilder : public OldSmbaseString {
protected:
  enum { EXTRA_SPACE = 30 };    // extra space allocated in some situations
  char *end;          // current end of the string (points to the NUL character)
  int size;           // amount of space (in bytes) allocated starting at 's'

protected:
  void init(int initSize);
  void dup(char const *src);

public:
  explicit stringBuilder(int length=0);    // creates an empty string
  explicit stringBuilder(char const *str);
           stringBuilder(char const *str, int length);
  explicit stringBuilder(OldSmbaseString const &str)
    : OldSmbaseString() { dup(str.c_str()); }
  stringBuilder(stringBuilder const &obj)
    : OldSmbaseString() { dup(obj.c_str()); }
  ~stringBuilder() {}

  stringBuilder& operator= (char const *src);
  stringBuilder& operator= (OldSmbaseString const &s) { return operator= (s.c_str()); }
  stringBuilder& operator= (stringBuilder const &s) { return operator= (s.c_str()); }

  int length() const { return end-s; }
  bool isempty() const { return length()==0; }

  // This is a problem when I construct a string from a stringBuilder.
  // That's not too common, but neither is using this (somewhat
  // dangerous) method, so I'll try disabling it.
#if 0
  // unlike 'OldSmbaseString' above, I will allow stringBuilder to convert to
  // char const * so I can continue to use 'stringc' to build strings
  // for functions that accept char const *; this should not conflict
  // with std::string, since I am explicitly using a different class
  // (namely stringBuilder) when I use this functionality
  operator char const * () const { return c_str(); }
#endif

  // Allow implicit conversion to 'string' so perhaps I can keep
  // 'stringc' working after all.
  operator std::string () const { return str(); }

  stringBuilder& setlength(int newlen);    // change length, forget current data

  // make sure we can store 'someLength' non-null chars; grow if necessary
  void ensure(int someLength) { if (someLength >= size) { grow(someLength); } }

  // std::string compatibility name for ensure()
  void reserve(int someLength) { ensure(someLength); }

  // grow the string's length (retaining data); make sure it can hold at least
  // 'newMinLength' non-null chars
  void grow(int newMinLength);

  // this can be useful if you modify the string contents directly..
  // it's not really the intent of this class, though
  void adjustend(char* newend);

  // remove characters from the end of the string; 'newLength' must
  // be at least 0, and less than or equal to current length
  void truncate(int newLength);

  // make the string be the empty string, but don't change the
  // allocated space
  void clear() { adjustend(s); }

  // concatenation, which is the purpose of this class
  stringBuilder& operator+= (char const *tail);

  // useful for appending substrings or strings with NUL in them
  void append(char const *tail, int length);

  // append a given number of spaces; meant for contexts where we're
  // building a multi-line string; returns '*this'
  stringBuilder& indent(int amt);

  // sort of a mixture of Java compositing and C++ i/o strstream
  stringBuilder& operator << (rostring text) { return operator+=(text.c_str()); }
  stringBuilder& operator << (char const *text) { return operator+=(text); }
  stringBuilder& operator << (char c);
  stringBuilder& operator << (unsigned char c) { return operator<<((char)c); }
  stringBuilder& operator << (long long i);
  stringBuilder& operator << (unsigned long long i);
  stringBuilder& operator << (long i);
  stringBuilder& operator << (unsigned long i);
  stringBuilder& operator << (int i) { return operator<<((long)i); }
  stringBuilder& operator << (unsigned i) { return operator<<((unsigned long)i); }
  stringBuilder& operator << (short i) { return operator<<((long)i); }
  stringBuilder& operator << (unsigned short i) { return operator<<((long)i); }
  stringBuilder& operator << (double d);
  stringBuilder& operator << (void *ptr);     // inserts address in hex
  #ifndef LACKS_BOOL
    stringBuilder& operator << (bool b) { return operator<<((long)b); }
  #endif // LACKS_BOOL

  // useful in places where long << expressions make it hard to
  // know when arguments will be evaluated, but order does matter
  typedef stringBuilder& (*Manipulator)(stringBuilder &sb);
  stringBuilder& operator<< (Manipulator manip);

  // work around problems invoking non-const non-member funcs
  // on temporaries
  stringBuilder &myself() { return *this; }

  // compatibility with ostringstream
  std::string str() const;

  // stream readers
  friend istream& operator>> (istream &is, stringBuilder &sb)
    { sb.readline(is); return is; }
  void readall(istream &is) { readdelim(is, NULL); }
  void readline(istream &is) { readdelim(is, "\n"); }

  void readdelim(istream &is, char const *delim);

  // an experiment: hex formatting (something I've sometimes done by resorting
  // to sprintf in the past)
  class Hex {
  public:
    unsigned long value;

    Hex(unsigned long v) : value(v) {}
    Hex(Hex const &obj) : value(obj.value) {}
  };
  stringBuilder& operator<< (Hex const &h);
  #define SBHex stringBuilder::Hex
};


// ---------------------- misc utils ------------------------
// 'stringb' and 'stringbc' are now defined in stringb.h.
#if 0
// the real strength of this entire module: construct strings in-place
// using the same syntax as C++ iostreams.  e.g.:
//   puts(stringb("x=" << x << ", y=" << y));
#define stringb(expr) (stringBuilder().myself() << expr)

// explicit c_str() is annoying
#define stringbc(expr) (stringb(expr).c_str())
#endif // 0

// This macro allows strings to be constructed like:
//
//   stringc << 123 << " hi " << "there"
//
// but is not compatible with an ostringstream-based implementation so
// 'stringb' should be preferred.
#define stringc (stringBuilder().myself())


// experimenting with using toString as a general method for datatypes
string toString(int i);
string toString(unsigned i);
string toString(char c);
string toString(long i);
string toString(char const *str);
string toString(float f);


#endif // SMBASE_STR_H
