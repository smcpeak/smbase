// string-hash.cc
// Code for `string-hash` module.

// License: Unclear.  I (Scott) volunteer my contributions as public
// domain, but this isn't primarily my work.  I copied this from smbase
// `strhash.cc`.  I've preserved comments below regarding the origin,
// but it's incomplete because the stated web search now (2024-06-01)
// yields essentially no hits.  Of course, all of this just boils down
// to multiplying by 31 and adding, so there's not really much IP to
// worry about here.

#include "string-hash.h"               // this module

#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <cstring>                     // std::strlen


OPEN_NAMESPACE(smbase)


unsigned stringHash(char const *data, std::size_t size)
{
  // All arithmetic should be unsigned.
  unsigned char const *p =
    reinterpret_cast<unsigned char const *>(data);
  unsigned char const *end = p + size;

  /* An excellent string hashing function.
     Adapted from glib's g_str_hash().
     Investigation by Karl Nelson <kenelson@ece.ucdavis.edu>.
     Do a web search for "g_str_hash X31_HASH" if you want to know more. */
  /* update: this is the same function as that described in Kernighan and Pike,
     "The Practice of Programming", section 2.9 */
  unsigned h = 0;
  for (; p < end; ++p) {
    // original X31_HASH
    h = ( h << 5 ) - h + *p;       // h*31 + *p
  }
  return h;
}


unsigned stringHashNulTerm(char const *cstr)
{
  return stringHash(cstr, std::strlen(cstr));
}


CLOSE_NAMESPACE(smbase)


// EOF
