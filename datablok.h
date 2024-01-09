// datablok.h            see license.txt for copyright and terms of use
// arbitrary block of data
// Scott McPeak, 1998-2000  This file is public domain.

#ifndef DATABLOK_H
#define DATABLOK_H

#include <stddef.h>                    // NULL, size_t, ptrdiff_t

#include "str.h"                       // string

class DataBlock {
private:      // data
  unsigned char *data;         // data itself (may be NULL)
  size_t dataLen;              // length of data, starting at data[0]
  size_t allocated;            // amount of memory allocated at 'data'

  // invariants: 0 <= dataLen <= allocated
  //             (data==NULL) == (allocated==0)

private:      // static data
  // endpost: 'data' will be kept allocated with one extra byte at the
  // end, where an endpost byte is written.  thus, we have another
  // invariant:
  //             (data!=NULL) implies data[allocated] == endpost
  static unsigned char const endpost;

public:       // static data
  // Normally if we detect corrupted memory we abort().  But for
  // testing, allow a different function to be called.
  static void (*s_memoryCorruptionOverrideHandler)();

private:      // funcs
  // base ctor
  void init(size_t allocatedSize);

  // allocate a block of memory, writing endpost
  static unsigned char *allocate(size_t size);

  // shared by both copy constructors (actually, only one is the true
  // copy ctor...)
  void copyCtorShared(DataBlock const &obj);

  // shared ctor calls as a workaround for char casting problems
  void ctor(unsigned char const *srcData, size_t dataLen);
  void ctor(unsigned char const *srcData, size_t dataLen, size_t allocatedSize);

  // confirm that invariants are true
  void selfCheck() const;

  // check that 'endpost' is at the end of the array
  void checkEndpost() const;

public:       // funcs
  // make an empty datablock holder; when allocatedSize is 0, 'data'
  // is initially set to NULL
  explicit DataBlock(size_t allocatedSize = 0);

  // Make a copy of 'srcString' using 'setFromString()'.
  explicit DataBlock(char const *srcString);

  // make a copy of 'srcData', which is 'dataLen' bytes long
  DataBlock(unsigned char const *srcData, size_t dataLen)
    { ctor(srcData, dataLen); }
  DataBlock(char const *srcData, size_t dataLen)
    { ctor((unsigned char const*)srcData, dataLen); }

  // make a copy of 'srcData', which is 'dataLen' bytes long, in a buffer
  // that is 'allocatedSize' bytes long
  DataBlock(unsigned char const *srcData, size_t dataLen, size_t allocatedSize)
    { ctor(srcData, dataLen, allocatedSize); }
  DataBlock(char const *srcData, size_t dataLen, size_t allocatedSize)
    { ctor((unsigned char const*)srcData, dataLen, allocatedSize); }

  // copy data, allocate same amount as 'obj'
  DataBlock(DataBlock const &obj);

  // copy obj's contents; allocate either obj.getAllocated() or
  // minToAllocate, whichever is larger (this turns out to be a
  // common idiom)
  DataBlock(DataBlock const &obj, size_t minToAllocate);

  ~DataBlock();

  // selectors
  unsigned char const *getDataC() const { return data; }
  size_t getDataLen() const { return dataLen; }
  size_t getAllocated() const { return allocated; }

  // compares data length and data-length bytes of data
  bool dataEqual(DataBlock const &obj) const;

  // compares data, length, and allocation length
  bool allEqual(DataBlock const &obj) const;

  // SM, 1/24/99: with the coding of blokutil, I've finally clarified that
  // allocation length is a concern of efficiency, not correctness, and so
  // have changed the meaning of == to just look at data.  The old behavior
  // is available as 'allEqual()'.
  bool operator== (DataBlock const &obj) const
    { return dataEqual(obj); }
  bool operator!= (DataBlock const &obj) const
    { return !operator==(obj); }

  // Return a string containing 'dataLen' characters, some of which
  // might be NUL.
  string toString() const;

  // ---- mutators ----
  unsigned char *getData() { return data; }

  // asserts that 0 <= newLen <= allocated
  void setDataLen(size_t newLen);

  void setAllocated(size_t newAllocated);     // i.e. realloc

  // add a null ('\0') to the end; there must be sufficient allocated space
  void addNull();

  void changeDataLen(ptrdiff_t changeAmount)
    { setDataLen(getDataLen() + changeAmount); }

  // if 'allocated' is currently less than minAllocated, then
  // set 'allocated' to minAllocated (preserving existing contents)
  void ensureAtLeast(size_t minAllocated);

  // grows allocated data if necessary, whereas changeDataLen will throw
  // an exception if there isn't already enough allocated space
  void growDataLen(ptrdiff_t changeAmount);

  // Set the data to 'srcString', *including* its NUL terminator.  The
  // resulting block has length of 'strlen(srcString)+1'.
  void setFromString(char const *srcString);

  void setFromBlock(unsigned char const *srcData, size_t dataLen);
  void setFromBlock(char const *srcData, size_t dataLen)
    { setFromBlock((unsigned char const*)srcData, dataLen); }

  // causes data AND allocation length equality
  DataBlock& operator= (DataBlock const &obj);

  // convenient file read/write
  void writeToFile(char const *fname) const;
  void readFromFile(char const *fname);

  // for debugging, write a simple representation to stdout if label is
  // not NULL, the data is surrounded by '---'-style delimiters
  enum { DEFAULT_PRINT_BYTES = 16 };
  void print(char const *label = NULL,
             int bytesPerLine = DEFAULT_PRINT_BYTES) const;

  // does nothing; useful for two reasons:
  //   1. lets me define macros that expand to 'print' during debug
  //      and dontPrint during non-debug
  //   2. plays a vital role in a g++ bug workaround (g++ sucks!!)
  void dontPrint(char const *label = NULL,
                 int bytesPerLine = DEFAULT_PRINT_BYTES) const;

  // utility, defined here for no good reason:
  //
  // print 'length' bytes of 'data' in hex
  // blank-pad the output as if 'linelen' bytes were present
  static void printHexLine(unsigned char const *data, size_t length,
                           size_t lineLength);

  // print 'length' bytes of 'data', substituting 'unprintable' for bytes for
  // which 'isprint' is false
  static void printPrintableLine(unsigned char const *data, size_t length,
                                 char unprintable = '.');
};


// Unit test defined in datablok-test.cc.
void test_datablok();


#endif // DATABLOK_H
