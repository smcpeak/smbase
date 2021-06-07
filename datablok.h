// datablok.h            see license.txt for copyright and terms of use
// arbitrary block of data
// Scott McPeak, 1998-2000  This file is public domain.

#ifndef DATABLOK_H
#define DATABLOK_H

#include <stddef.h>                    // NULL

class DataBlock {
private:      // data
  unsigned char *data;         // data itself (may be NULL)
  int dataLen;                 // length of data, starting at data[0]
  int allocated;               // amount of memory allocated at 'data'

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
  void init(int allocatedSize);
    // base ctor

  static unsigned char *allocate(int size);
    // allocate a block of memory, writing endpost

  void copyCtorShared(DataBlock const &obj);
    // shared by both copy constructors (actually, only one is the true
    // copy ctor...)

  void ctor(unsigned char const *srcData, int dataLen);
  void ctor(unsigned char const *srcData, int dataLen, int allocatedSize);
    // shared ctor calls as a workaround for char casting problems

  void selfCheck() const;
    // confirm that invariants are true

  void checkEndpost() const;
    // check that 'endpost' is at the end of the array

public:       // funcs
  // constructors
  DataBlock(int allocatedSize = 0);
    // make an empty datablock holder; when allocatedSize is 0, 'data'
    // is initially set to NULL

  explicit DataBlock(char const *srcString);
    // make a copy of 'srcString' data, which is null-terminated

  DataBlock(unsigned char const *srcData, int dataLen)
    { ctor(srcData, dataLen); }
  DataBlock(char const *srcData, int dataLen)
    { ctor((unsigned char const*)srcData, dataLen); }
    // make a copy of 'srcData', which is 'dataLen' bytes long

  DataBlock(unsigned char const *srcData, int dataLen, int allocatedSize)
    { ctor(srcData, dataLen, allocatedSize); }
  DataBlock(char const *srcData, int dataLen, int allocatedSize)
    { ctor((unsigned char const*)srcData, dataLen, allocatedSize); }
    // make a copy of 'srcData', which is 'dataLen' bytes long, in a buffer
    // that is 'allocatedSize' bytes long

  DataBlock(DataBlock const &obj);
    // copy data, allocate same amount as 'obj'

  DataBlock(DataBlock const &obj, int minToAllocate);
    // copy obj's contents; allocate either obj.getAllocated() or
    // minToAllocate, whichever is larger (this turns out to be a
    // common idiom)

  ~DataBlock();

  // selectors
  unsigned char const *getDataC() const { return data; }
  int getDataLen() const { return dataLen; }
  int getAllocated() const { return allocated; }

  bool dataEqual(DataBlock const &obj) const;
    // compares data length and data-length bytes of data

  bool allEqual(DataBlock const &obj) const;
    // compares data, length, and allocation length

  bool operator== (DataBlock const &obj) const
    { return dataEqual(obj); }
  bool operator!= (DataBlock const &obj) const
    { return !operator==(obj); }
    // SM, 1/24/99: with the coding of blokutil, I've finally clarified that
    // allocation length is a concern of efficiency, not correctness, and so
    // have changed the meaning of == to just look at data.  The old behavior
    // is available as 'allEqual()'.

  // mutators
  unsigned char *getData() { return data; }
  void setDataLen(int newLen);
    // asserts that 0 <= newLen <= allocated
  void setAllocated(int newAllocated);     // i.e. realloc
  void addNull();
    // add a null ('\0') to the end; there must be sufficient allocated space

  void changeDataLen(int changeAmount)
    { setDataLen(getDataLen() + changeAmount); }

  void ensureAtLeast(int minAllocated);
    // if 'allocated' is currently less than minAllocated, then
    // set 'allocated' to minAllocated (preserving existing contents)

  void growDataLen(int changeAmount);
    // grows allocated data if necessary, whereas changeDataLen will throw
    // an exception if there isn't already enough allocated space

  void setFromString(char const *srcString);
  void setFromBlock(unsigned char const *srcData, int dataLen);
  void setFromBlock(char const *srcData, int dataLen)
    { setFromBlock((unsigned char const*)srcData, dataLen); }

  DataBlock& operator= (DataBlock const &obj);
    // causes data AND allocation length equality

  // convenient file read/write
  void writeToFile(char const *fname) const;
  void readFromFile(char const *fname);

  // for debugging
  enum { DEFAULT_PRINT_BYTES = 16 };
  void print(char const *label = NULL,
             int bytesPerLine = DEFAULT_PRINT_BYTES) const;
    // write a simple representation to stdout
    // if label is not NULL, the data is surrounded by '---'-style delimiters

  void dontPrint(char const *label = NULL,
                 int bytesPerLine = DEFAULT_PRINT_BYTES) const;
    // does nothing; useful for two reasons:
    //   1. lets me define macros that expand to 'print' during debug
    //      and dontPrint during non-debug
    //   2. plays a vital role in a g++ bug workaround (g++ sucks!!)

  // utility, defined here for no good reason
  static void printHexLine(unsigned char const *data, int length, int lineLength);
    // print 'length' bytes of 'data' in hex
    // blank-pad the output as if 'linelen' bytes were present

  static void printPrintableLine(unsigned char const *data, int length,
                                 char unprintable = '.');
    // print 'length' bytes of 'data', substituting 'unprintable' for bytes for
    // which 'isprint' is false
};


// Unit test defined in test-datablok.cc.
void test_datablok();


#endif // DATABLOK_H
