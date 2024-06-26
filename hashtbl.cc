// hashtbl.cc            see license.txt for copyright and terms of use
// code for hashtbl.h

#include "hashtbl.h"                   // this module

#include "pointer-util.h"              // pointerToInteger
#include "sm-test.h"                   // PVALTO
#include "xassert.h"                   // xassert

#include <iostream>                    // std::ostream

#include <string.h>                    // memset


unsigned HashTable::hashFunction(void const *key) const
{
  return coreHashFn(key) % (unsigned)tableSize;
}


HashTable::HashTable(GetKeyFn gk, HashFn hf, EqualKeyFn ek, int initSize)
  : getKey(gk),
    coreHashFn(hf),
    equalKeys(ek),
    enableShrink(true)
{
  makeTable(initSize);
}

HashTable::~HashTable()
{
  delete[] hashTable;
}


void HashTable::makeTable(int size)
{
  hashTable = new void*[size];
  tableSize = size;
  memset(hashTable, 0, sizeof(void*) * tableSize);
  numEntries = 0;
}


inline int HashTable::getEntry(void const *key) const
{
  int index = hashFunction(key);
  int originalIndex = index;
  for(;;) {
    if (hashTable[index] == NULL) {
      // unmapped
      return index;
    }
    if (equalKeys(key, getKey(hashTable[index]))) {
      // mapped here
      return index;
    }

    // this entry is mapped, but not with this key, i.e.
    // we have a collision -- so just go to the next entry,
    // wrapping as necessary
    index = nextIndex(index);

    // detect infinite looping
    xassert(index != originalIndex);
  }
}


void *HashTable::get(void const *key) const
{
  return hashTable[getEntry(key)];
}


void HashTable::resizeTable(int newSize)
{
  // Make sure newSize is not the result of an overflowed computation, and
  // that we're not going to resizeTable again right away in the add() call.
  xassert(newSize >= numEntries);
  xassert(newSize/3*2 >= numEntries-1);

  // save old stuff
  void **oldTable = hashTable;
  int oldSize = tableSize;
  int oldEntries = numEntries;

  // make the new table (sets 'numEntries' to 0)
  makeTable(newSize);

  // set this now, rather than incrementing it with each add
  numEntries = oldEntries;

  // move entries to the new table
  for (int i=0; i<oldSize; i++) {
    if (oldTable[i] != NULL) {
      // This function is worth optimizing; it accounts for 12% of qualcc
      // runtime.  This simple inlining reduces resizeTable()'s impact from
      // 12% to 2%.
      if (0) {
        // original code:
        add(getKey(oldTable[i]), oldTable[i]);
      }
      else {
        // inlined version:
        int newIndex = getEntry(getKey(oldTable[i]));
        xassertdb(hashTable[newIndex] == NULL);    // must not be a mapping yet
        hashTable[newIndex] = oldTable[i];
      }

      oldEntries--;
    }
  }
  xassert(oldEntries == 0);

  // deallocate the old table
  delete[] oldTable;
}


void HashTable::add(void const *key, void *value)
{
  if (numEntries+1 > tableSize*2/3) {
    // We're over the usage threshold; increase table size.
    //
    // Note: Resizing by numEntries*4 instead of tableSize*2 gives 42% speedup
    // in total time used by resizeTable(), at the expense of more memory used.
    resizeTable(tableSize * 2 + 1);
  }
  // make sure above didn't fail due to integer overflow
  xassert(numEntries+1 < tableSize);

  int index = getEntry(key);
  xassert(hashTable[index] == NULL);    // must not be a mapping yet

  hashTable[index] = value;
  numEntries++;
}


void *HashTable::remove(void const *key)
{
  if (enableShrink                &&
      numEntries-1 < tableSize/5  &&
      tableSize > defaultSize) {
    // we're below threshold; reduce table size
    resizeTable(tableSize / 2);
  }

  int index = getEntry(key);
  xassert(hashTable[index] != NULL);    // must be a mapping to remove

  // remove this entry
  void *retval = hashTable[index];
  hashTable[index] = NULL;
  numEntries--;

  // now, if we ever inserted something and it collided with this one,
  // leaving things like this would prevent us from finding that other
  // mapping because the search stops as soon as a NULL entry is
  // discovered; so we must examine all entries that could have
  // collided, and re-insert them
  int originalIndex = index;
  for(;;) {
    index = nextIndex(index);
    xassert(index != originalIndex);    // prevent infinite loops

    if (hashTable[index] == NULL) {
      // we've reached the end of the list of possible colliders
      break;
    }

    // remove this one
    void *data = hashTable[index];
    hashTable[index] = NULL;
    numEntries--;

    // add it back
    add(getKey(data), data);
  }

  return retval;
}


void HashTable::empty(int initSize)
{
  delete[] hashTable;
  makeTable(initSize);
}


void HashTable::printStats(std::ostream &os) const
{
  PVALTO(os, tableSize);
  PVALTO(os, numEntries);
}


void HashTable::selfCheck() const
{
  int ct=0;
  for (int i=0; i<tableSize; i++) {
    if (hashTable[i] != NULL) {
      checkEntry(i);
      ct++;
    }
  }

  xassert(ct == numEntries);
}

void HashTable::checkEntry(int entry) const
{
  int index = getEntry(getKey(hashTable[entry]));
  int originalIndex = index;
  for(;;) {
    if (index == entry) {
      // the entry lives where it will be found, so that's good
      return;
    }
    if (hashTable[index] == NULL) {
      // the search for this entry would stop before finding it,
      // so that's bad!
      xfailure("checkEntry: entry in wrong slot");
    }

    // collision; keep looking
    index = nextIndex(index);
    xassert(index != originalIndex);
  }
}


// ------------------ HashTableIter --------------------
HashTableIter::HashTableIter(HashTable &t)
  : table(t)
{
  index = 0;
  moveToSth();
}

void HashTableIter::adv()
{
  xassert(!isDone());

  // move off the current item
  index++;

  // keep moving until we find something
  moveToSth();
}

void HashTableIter::moveToSth()
{
  while (index < table.tableSize &&
         table.hashTable[index] == NULL) {
    index++;
  }

  if (index == table.tableSize) {
    index = -1;    // mark as done
  }
}


void *HashTableIter::data() const
{
  xassert(!isDone());
  return table.hashTable[index];
}


STATICDEF void const *HashTable::identityKeyFn(void *data)
{
  return data;
}

unsigned lcprngTwoSteps(unsigned v)
{
  // this is the core of the LC PRNG in one of the many libcs
  // running around the net
  v = (v * 1103515245) + 12345;

  // do it again for good measure
  v = (v * 1103515245) + 12345;

  return v;
}

STATICDEF unsigned HashTable::lcprngHashFn(void const *key)
{
  return lcprngTwoSteps((unsigned)pointerToInteger(key));
}

STATICDEF bool HashTable::
  pointerEqualKeyFn(void const *key1, void const *key2)
{
  return key1 == key2;
}
