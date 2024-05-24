// vptrmap.cc
// code for vptrmap.h

#include "vptrmap.h"                   // this module

#include "pointer-utils.h"             // pointerToInteger
#include "xassert.h"                   // xfailure

#include <stddef.h>                    // NULL
#include <string.h>                    // memset


// ------------------ VoidPtrMap -------------------
int VoidPtrMap::lookups = 0;
int VoidPtrMap::probes = 0;


VoidPtrMap::VoidPtrMap()
  : hashTable(NULL),
    tableSize(0),
    tableSizeBits(0),
    numEntries(0),
    iterators(0)
{
  alloc(4);    // 16 entries initially
  empty();
}

VoidPtrMap::~VoidPtrMap()
{
  delete[] hashTable;
}


void VoidPtrMap::alloc(int bits)
{
  tableSizeBits = bits;
  tableSize = 1 << bits;
  hashTable = new Entry[tableSize];
}


inline unsigned VoidPtrMap::hashFunc(unsigned multiplier, unsigned key) const
{
  // see Cormen/Leiserson/Rivest (CLR), section 12.3.2

  // multiply, throwing away the overflow high bits
  unsigned ret = key * multiplier;

  // we want to extract the 'tableSizeBits' most sigificant bits
  ret = ret >> ((sizeof(unsigned)*8) - tableSizeBits);
  ret = ret & (tableSize-1);

  return ret;
}


VoidPtrMap::Entry &VoidPtrMap::findEntry(void const *key) const
{
  xassert(key != NULL);
  lookups++;

  // constants used in the hash functions
  enum {
    // value is  floor(  (sqrt(5)-1)/2 * 2^32  )
    //
    // This is the golden ratio.  CLR says Knuth says it's good.
    CONST1 = 0x9E3779B9U,

    // value is  floor(  (sqrt(3)-1)/2 * 2^32  )
    //
    // Some random website claims irrational constants are good,
    // and I can't find any source (I don't have Knuth..) for
    // another constant, so I just decided to substitute 3 for
    // 5 in the golden ratio formula.  Since I trust this one
    // less, I use it for the less important role (stride).
    CONST2 = 0x5DB3D742U
  };

  // compute first hash function, which gives the starting index
  // for the probe sequence
  unsigned index = hashFunc(CONST1, (unsigned)pointerToInteger(key));

  // analyze the first entry now, before computing the second
  // hash function (stride) value
  {
    probes++;
    Entry &e = hashTable[index];
    if (e.key == NULL ||
        e.key == key) {
      return e;
    }
  }

  // compute stride; it has to be odd so that it is relatively
  // prime to the table size (which is a power of 2), so I just
  // turn on the least significant bit
  unsigned stride = hashFunc(CONST2, (unsigned)pointerToInteger(key)) | 1;

  // uncomment this to experiment with linear hashing; when ITERS2MAX
  // is 10000, I see a small increase in avgprobes when using linear
  // hashing over double hashing
  //unsigned stride = 1;

  // collision; stride over the entries
  for (int i=0; i<tableSize; i++) {
    index = (index + stride) & (tableSize-1);

    probes++;
    Entry &e = hashTable[index];
    if (e.key == NULL ||
        e.key == key) {
      return e;
    }
  }

  // searched all entries with no success; but if this happens,
  // then our load factor must be 1, which violates the invariant
  // that numEntries < tableSize
  xfailure("findEntry traversed all entries");
  return *((Entry*)NULL);     // silence warning
}


void VoidPtrMap::add(void *key, void *value)
{
  xassert(iterators == 0);

  // if load factor would exceed 3/4, expand
  if (numEntries+1 > (tableSize/2 + tableSize/4)) {
    expand();
  }

  Entry &e = findEntry(key);
  if (e.key == NULL) {
    e.key = key;              // new mapping
    numEntries++;
  }
  else {
    xassert(e.key == key);    // update existing mapping
  }
  e.value = value;
}


void VoidPtrMap::expand()
{
  Entry *oldHashTable = hashTable;
  int oldTableSize = tableSize;

  alloc(tableSizeBits + 1);
  empty();

  // re-insert all of the old elements
  for (int i=0; i < oldTableSize; i++) {
    Entry &e = oldHashTable[i];
    if (e.key) {
      add(e.key, e.value);
    }
  }

  delete[] oldHashTable;
}


void VoidPtrMap::empty()
{
  xassert(iterators == 0);

  // establishes invariant that NULL keys have NULL values
  memset(hashTable, 0, sizeof(*hashTable) * tableSize);
  numEntries = 0;
}


// ------------------- VoidPtrMap::Iter ------------------
VoidPtrMap::Iter::Iter(VoidPtrMap const &m)
  : map(m),
    index(map.tableSize)
{
  map.iterators++;
  adv();
}

VoidPtrMap::Iter::~Iter()
{
  map.iterators--;
}


void VoidPtrMap::Iter::adv()
{
  xassert(index >= 0);
  index--;
  while (index >= 0 &&
         map.hashTable[index].key == NULL) {
    index--;
  }
}


// EOF
