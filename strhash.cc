// strhash.cc            see license.txt for copyright and terms of use
// code for strhash.h

#include "strhash.h"     // this module
#include "xassert.h"     // xassert

#include <string.h>      // strcmp

// notes on string hash functions ****************

// We need to test the various hash functions versus each other for
// randomness.  Scott suggests simply hashing the same data and
// modding it down as a hashtable would and seeing which gives more
// collisions.

// Note that both hash functions could be improved if they were aware
// of how many of their low order bits were really going to be
// preserved and then taking the high order bits that would otherwise
// be discarded and xoring or adding them to the low order bits.

// Hash function 2:

// 1) Does not work on architectures other than 32 bit.

// 2) Will not work well or at all if the strings do not start on
// 32-bit aligned boundaries.

// 3) Could be made to work faster if the strings start AND end on
// 32-bit aligned boundaries and are padded out with NUL-s.  Actually,
// if we do this trick, then the code becomes portable with no
// #ifdef-s !  All you do is cast the array to an array of ints and
// then test for termination by masking off all but the last 8 bits.
// Everything else is just operations on ints.  You might want to pick
// 64-bit primes, but they will work in 32-bit mode as long as the
// compiler just truncates their high bits off.

// ****************


StringHash::StringHash(GetKeyFn gk)
  : HashTable((HashTable::GetKeyFn)gk,
              (HashTable::HashFn)coreHash,
              (HashTable::EqualKeyFn)keyCompare)
{}

StringHash::~StringHash()
{}


STATICDEF unsigned StringHash::coreHash(char const *key)
{
  // dsw: not sure if this is the best place for it, but an assertion
  // failure is better than a segfault
  //
  // sm: I don't agree; segfaults arising from NULL derefs are
  // quite friendly (deref'ing random address is another story).
  // Anyway, this is fine, but I'd like it to go away in NDEBUG
  // mode so I'm changing it to use 'xassertdb'.
  xassertdb(key);

  // some more references:

  // http://www.cs.yorku.ca/~oz/hash.html
  //
  // Describes three well-known hashes: djb2, sdbm, and K&R ed. 1.

  // http://burtleburtle.net/bob/hash/doobs.html
  //
  // Describes a particular hash function (called simply "My Hash")
  // and provides justifications for preferring it to several others,
  // including MD4.

  // http://www.isthe.com/chongo/tech/comp/fnv/
  //
  // Glen Fowler, Landon Curt Noll, and Phong Vo's hash function.

  // http://mail.python.org/pipermail/python-dev/2004-April/044235.html
  //
  // Start of a discussion thread about changing the string hash
  // in Python.






  #if 0
  // I pulled this out of my ass.. it's supposed to mimic
  // a linear congruential random number generator
  unsigned val = 0x42e9d115;    // arbitrary
  while (*key != 0) {
    val *= (unsigned)(*key);
    val += 1;
    key++;
  }
  return val;
  #endif // 0

  // pick a default STRHASH_ALG
  #ifndef STRHASH_ALG
    #define STRHASH_ALG 1
  #endif // STRHASH_ALG


  #if STRHASH_ALG == 1
  #ifdef SAY_STRHASH_ALG
    #warning hash function 1: Nelson
  #endif // SAY_STRHASH_ALG
  // this one is supposed to be better
  /* An excellent string hashing function.
     Adapted from glib's g_str_hash().
     Investigation by Karl Nelson <kenelson@ece.ucdavis.edu>.
     Do a web search for "g_str_hash X31_HASH" if you want to know more. */
  /* update: this is the same function as that described in Kernighan and Pike,
     "The Practice of Programming", section 2.9 */
  unsigned h = 0;
  for (; *key != '\0'; key += 1) {
    // original X31_HASH
    h = ( h << 5 ) - h + *key;       // h*31 + *key

    // dsw: this one is better because it does the multiply last;
    // otherwise the last byte has no hope of modifying the high order
    // bits
    //
    // sm: I'm not convinced it's better.  For short strings, say less
    // than 6 characters, the arithmetic won't overflow a 32-bit
    // register.  In that case, by multiplying last, the hash value is
    // always a multiple of 31 and hence will suffer many more
    // collisions.  I would like more justification in the form of
    // experimental measurements before making a change.
    //h += *key;
    //h = ( h << 5 ) - h;         // h *= 31
  }
  return h;


  #elif STRHASH_ALG == 2
  #ifdef SAY_STRHASH_ALG
    #warning hash function 2: word-rotate/final-mix
  #endif // SAY_STRHASH_ALG

  // FIX:
  #warning word-rotate/final-mix hash function only works on 32-bit architectures
  #warning word-rotate/final-mix hash function still needs to be tested for randomness vs nelson

  // Word-Rotate / Final-Mix hash function by Daniel Wilkerson; A
  // slighly faster and likely more random hash function; Invented in
  // collaboration with Simon Goldsmith.
  //
  // Supposedly gcc will sometimes recognize this and generate a
  // single rotate instruction 'ROR'.  Thanks to Matt Harren for this.
  // http://groups.google.com/groups?q=rorl+x86&start=10&hl=en&lr=&ie=UTF-8&oe=UTF-8&
  // selm=359954C9.3B354F0%40cartsys.com&rnum=11
  // http://www.privacy.nb.ca/cryptography/archives/coderpunks/new/1998-10/0096.html
  #define ROTATE(n, b) (n >> b) | (n << (32 - b))

  // Note that UINT_MAX        = 4294967295U;
  // source of primes: http://www.utm.edu/research/primes/lists/small/small.html
  static unsigned const primeA = 1500450271U;
  static unsigned const primeB = 2860486313U;
  // source of primes: http://www.utm.edu/research/primes/lists/2small/0bit.html
  static unsigned const primeC = (1U<<31) - 99U;
  static unsigned const primeD = (1U<<30) - 35U;

  static int count = 0;
  ++count;

  unsigned h = primeA;

  // Stride a word at a time.  Note that this works best (or works at
  // all) if the string is 32-bit aligned.  This initial 'if' block is
  // to prevent an extra unneeded rotate.
  //
  // FIX: this would be even faster if all strings were NUL-padded in
  // length to a multiple of 4; we could then omit all but the last
  // 'if' and the ragged end after the loop (it doesn't matter if you
  // tile a few extra NULs into your value).
  if (!key[0]) goto end0;
  if (!key[1]) goto end1;
  if (!key[2]) goto end2;
  if (!key[3]) goto end3;
  // No rotate here.
  h += *( (unsigned *) key );
  key += 4;
  while (1) {
    // invariant: when we get here, we are ready to rotate
    if (!key[0]) {h = ROTATE(h, 5); goto end0;}
    if (!key[1]) {h = ROTATE(h, 5); goto end1;}
    if (!key[2]) {h = ROTATE(h, 5); goto end2;}
    if (!key[3]) {h = ROTATE(h, 5); goto end3;}
    h = ROTATE(h, 5);
    // FIX: if the start of the string is not 4-byte aligned then this
    // will be slower on x86 and I think even illegal on MIPS and
    // perhaps others.  To be portable we should ensure this.
    h += *( (unsigned *) key ); // on my machine plus is faster than xor
    key += 4;
  }
  xfailure("shouldn't get here");

  // deal with the ragged end
  // invariant: when we get here we are ready to add
end3:
  h += *key; h = ROTATE(h, 5); key += 1;
end2:
  h += *key; h = ROTATE(h, 5); key += 1;
end1:
  h += *key;                    // No rotate nor increment here.
  #ifndef NDEBUG
    key += 1;                   // this is only needed for the assertion below
  #endif
end0:
  xassertdb(*key=='\0');

  // I will say for now that the property of hash functions that we
  // want is that a change in any input bit has a 50% chance of
  // inverting any output bit.
  //
  // At this point, compare this hash function to the Nelson hash
  // function above.  In Nelson, everytime data is added, the
  // accumulator, 'h', is "stirred" with a multiplication.  Since data
  // is being added at the low byte and since multiplication
  // propagates dependencies towards the high bytes and since after
  // ever add there is at least one multiply, every byte has a chance
  // to affect the value of every bit above the first byte.
  //
  // However, in this hash function we have saved time by simply
  // "tiling" the data across the accumulator and at this point it
  // hasn't been "stirred" at all.  Since most hashvalues are used by
  // modding off some high bits, those bits have never had a chance to
  // affect the final value, so some stirring is needed.  How much
  // "stirring" do we need?
  //
  // Consider the 32-bit word in two halves, H and L.
  //
  // 1) With a single multiply, any bit of L "has a chance" to affect
  // any bit in H.  The reverse is not true.
  h *= primeB;

  // 2) We therefore swap H and L and multiply again.  Now, any bit in
  // H has had a chance to affect any bit in L.  We are not done
  // though, since the high order bits in H have not had a chance to
  // affect the low order bits of H (yes H).  Please note however,
  // that since L affected H and H affected L, the high order bits of
  // L *have* had a chance to affect the low order bits of L.
  h = ROTATE(h, 16);
  h *= primeC;

  // 3) Therefore we swap H and L and multiply once again.  Now the
  // high order bits of H have had a chance to affect L (in 2) which
  // now can affect H again.  Any bit now has "had a chance" to affect
  // any other bit.
  h = ROTATE(h, 16);
  h *= primeD;

  return h;

  #undef ROTATE


  #else
    #error You must pick a hash function
  #endif // STRHASH_ALG multi-switch
}


STATICDEF bool StringHash::keyCompare(char const *key1, char const *key2)
{
  return 0==strcmp(key1, key2);
}


int const strhashAlgorithmCode = STRHASH_ALG;


// EOF
