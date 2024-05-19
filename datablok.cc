// datablok.cc            see license.txt for copyright and terms of use
// code for datablok.h
// Scott McPeak, 1998-2000  This file is public domain.

#include "datablok.h"     // this module
#include "exc.h"          // xassert, CAUTIOUS_RELAY
#include "crc.h"          // crc32
#include "syserr.h"       // xsyserror

#include <limits.h>       // INT_MAX
#include <stdio.h>        // printf
#include <stdlib.h>       // abort
#include <string.h>       // memcpy
#include <ctype.h>        // isprint


// define the endpost byte as something we hope is
// unlikely to coincidentally be written during an
// overrun
/*static*/ unsigned char const DataBlock::endpost = 0xBB;

/*static*/ void (*DataBlock::s_memoryCorruptionOverrideHandler)() = NULL;


void DataBlock::init(size_t allocatedSize)
{
  xassert(allocatedSize >= 0);
  dataLen = 0;
  allocated = allocatedSize;
  if (allocated) {
    data = allocate(allocated);
  }
  else {
    data = NULL;
  }

  SELFCHECK();
}


STATICDEF unsigned char *DataBlock::allocate(size_t size)
{
  unsigned char *ret = new unsigned char[size+1];
  ret[size] = endpost;
  return ret;
}


void DataBlock::selfCheck() const
{
  this->checkEndpost();
  xassert(0 <= dataLen && dataLen <= allocated);
  xassert( (data==NULL) == (allocated==0) );
}


void DataBlock::checkEndpost() const
{
  // Check for memory corruption.
  if (data != NULL && data[allocated] != endpost) {
    fprintf(stderr, "DataBlock: array overrun detected!\n"
                    "  this: %p\n"
                    "  data: %p\n"
                    "  allocated: %zu\n"
                    "  dataLen: %zu\n"
                    "  data[allocated]: %d\n"
                    "Program will now terminate.\n",
                    this,
                    data,
                    allocated,
                    dataLen,
                    (int)data[allocated]);
    fflush(stderr);

    if (s_memoryCorruptionOverrideHandler) {
      (*s_memoryCorruptionOverrideHandler)();
    }
    else {
      // Memory corruption is not recoverable.
      abort();
    }
  }
}


DataBlock::DataBlock(size_t allocatedSize)
{
  init(allocatedSize);
  SELFCHECK();
}


DataBlock::DataBlock(char const *srcString)
{
  init(0);
  setFromString(srcString);
  SELFCHECK();
}


void DataBlock::ctor(unsigned char const *srcData, size_t dataLen)
{
  init(0);
  setFromBlock(srcData, dataLen);
  SELFCHECK();
}


void DataBlock::ctor(unsigned char const *srcData, size_t srcDataLen, size_t allocatedSize)
{
  init(allocatedSize);
  dataLen = srcDataLen;
  memcpy(data, srcData, dataLen);
  SELFCHECK();
}


DataBlock::DataBlock(DataBlock const &obj)
{
  init(obj.allocated);
  copyCtorShared(obj);
}

void DataBlock::copyCtorShared(DataBlock const &obj)
{
  dataLen = obj.dataLen;
  if (dataLen > 0) {
    memcpy(data, obj.data, dataLen);
  }
  SELFCHECK();
}


DataBlock::DataBlock(DataBlock const &obj, size_t minToAllocate)
{
  init(max(obj.getAllocated(), minToAllocate));
  copyCtorShared(obj);
}


DataBlock::~DataBlock()
{
  // Do not do a full self-check, since that might throw an
  // exception.  But do check for memory corruption, which
  // will abort if there is a problem.
  this->checkEndpost();

  if (data) {
    delete[] data;
  }
}


bool DataBlock::allEqual(DataBlock const &obj) const
{
  SELFCHECK();
  return allocated == obj.allocated &&
         dataEqual(obj);
}


bool DataBlock::dataEqual(DataBlock const &obj) const
{
  SELFCHECK();
  if (dataLen != obj.dataLen ||
      (dataLen > 0 &&
       0 != memcmp(data, obj.data, dataLen))) {
    return false;
  }
  else {
    return true;
  }
}


OldSmbaseString DataBlock::toString() const
{
  // My 'OldSmbaseString' class uses 'int' for its length.  That should be fixed,
  // but until then, I'll verify the conversion is safe.
  xassert(getDataLen() <= INT_MAX);

  return OldSmbaseString((char*)getDataC(), getDataLen());
}


void DataBlock::setDataLen(size_t newLen)
{
  SELFCHECK();
  xassert(0 <= newLen && newLen <= allocated);
  dataLen = newLen;
  SELFCHECK();
}


void DataBlock::setAllocated(size_t newAllocated)
{
  SELFCHECK();
  xassert(newAllocated >= 0);
  if (allocated != newAllocated) {
    // allocate new buffer
    unsigned char *newData = NULL;
    if (newAllocated > 0) {
      newData = allocate(newAllocated);
    }

    // truncate defined data
    if (dataLen > newAllocated) {
      dataLen = newAllocated;
    }

    // transfer data
    if (dataLen > 0) {
      memcpy(newData, data, dataLen);
    }

    // deallocate old buffer and replace with new buffer
    delete[] data;
    data = newData;
    allocated = newAllocated;
  }
  SELFCHECK();
}


void DataBlock::ensureAtLeast(size_t minAllocated)
{
  if (allocated < minAllocated) {
    setAllocated(minAllocated);
  }
}


void DataBlock::growDataLen(ptrdiff_t changeAmount)
{
  ensureAtLeast(getDataLen() + changeAmount);
  changeDataLen(changeAmount);
}


void DataBlock::addNull()
{
  SELFCHECK();
  data[dataLen] = 0;
  setDataLen(dataLen + 1);
  SELFCHECK();
}


void DataBlock::setFromString(char const *srcString)
{
  SELFCHECK();
  size_t len = strlen(srcString)+1;
    // a string is its contents and the null terminator
  setFromBlock((unsigned char const*)srcString, len);
  SELFCHECK();
}

void DataBlock::setFromBlock(unsigned char const *srcData, size_t len)
{
  SELFCHECK();
  if (len > allocated) {
    setAllocated(len);
  }
  setDataLen(len);
  if (len > 0) {
    memcpy(data, srcData, len);
  }
  SELFCHECK();
}


DataBlock& DataBlock::operator= (DataBlock const &obj)
{
  SELFCHECK();
  if (this != &obj) {
    setAllocated(obj.allocated);
    dataLen = obj.dataLen;
    memcpy(data, obj.data, dataLen);
  }
  SELFCHECK();
  return *this;
}


void DataBlock::print(char const *label, int bytesPerLine) const
{
  xassert(bytesPerLine >= 1);
  SELFCHECK();

  if (label) {
    printf("---- %s, length = %zu, crc32 = 0x%lX ---- {\n",
           label, getDataLen(),
           (unsigned long)crc32(getDataC(), getDataLen()));
  }

  size_t cursor = 0;
  while (cursor < getDataLen()) {
    int linelen = (int)min((size_t)bytesPerLine, getDataLen() - cursor);
    xassert(linelen >= 1);    // ensure can't loop infinitely

    printf("  ");     // indent
    printHexLine(getDataC() + cursor, linelen, bytesPerLine);
    printf("   ");
    printPrintableLine(getDataC() + cursor, linelen);
    printf("\n");

    cursor += linelen;
  }

  if (label) {
    printf("}\n");
  }

  fflush(stdout);
  SELFCHECK();
}


// print 'length' bytes of 'data' in hex
// blank-pad the output as if 'linelen' bytes were present
STATICDEF void DataBlock::printHexLine(unsigned char const *data,
                                       size_t length, size_t linelen)
{
  xassert(data != NULL &&
          length >= 1 &&
          linelen >= length);

  for (size_t i=0; i<linelen; i++) {
    if (i < length) {
      printf("%02X ", *data);
      data++;
    }
    else {
      printf("   ");
    }
  }
}


// print 'length' bytes of 'data', substituting 'unprintable' for bytes for
// which 'isprint' is false
STATICDEF void DataBlock::printPrintableLine(unsigned char const *data,
                                             size_t length, char unprintable)
{
  xassert(data != NULL &&
          length >= 1);

  while (length--) {
    if (isprint(*data)) {
      printf("%c", *data);
    }
    else {
      printf("%c", unprintable);
    }
    data++;
  }
}


#if 0
void DataBlock::print(char const *label) const
{
  enum { MARGIN = 70 };

  if (label) {
    printf("------ %s (length=%d) -------\n", label, getDataLen());
  }

  unsigned char *p = data;
  size_t i;
  size_t column=0;
  for (i=0; i<dataLen; i++, p++) {
    if (isprint(*p)) {
      if (*p != '\\') {
        column += printf("%c", *p);
      }
      else {
        printf("\\\\");     // otherwise '\\','x','nn','nn' would be ambiguous
      }
    }
    else {
      column += printf("\\x%02X", *p);
    }

    if (column >= MARGIN && (i+1) < dataLen) {
      printf("\\\n");       // continuation lines end with backslash
      column = 0;
    }
  }

  // this makes spaces at the end of a buffer invisible.. oh well..
  if (column != 0) {    // if didn't just newline...
    printf("\n");
  }

  if (label) {
    printf("------ end of %s -------\n", label);
  }
}
#endif // 0


void DataBlock::dontPrint(char const *, int) const
{}


void DataBlock::writeToFile(char const *fname) const
{
  FILE *fp = fopen(fname, "wb");
  if (!fp) {
    xsyserror("fopen", fname);
  }

  if (fwrite(getDataC(), 1, getDataLen(), fp) != getDataLen()) {
    xsyserror("fwrite", fname);
  }

  if (fclose(fp) != 0) {
    xsyserror("fclose", fname);
  }
}


void DataBlock::readFromFile(char const *fname)
{
  FILE *fp = fopen(fname, "rb");
  if (!fp) {
    xsyserror("fopen", fname);
  }

  // seek to end to know how much to allocate
  if (fseek(fp, 0, SEEK_END) != 0) {
    xsyserror("fseek", fname);
  }

  long len = ftell(fp);
  if (len < 0) {
    xsyserror("ftell", fname);
  }

  setAllocated(len);

  // read data
  if (fseek(fp, 0, SEEK_SET) != 0) {
    xsyserror("fseek", fname);
  }

  if ((long)fread(getData(), 1, len, fp) != len) {
    xsyserror("fread", fname);
  }

  setDataLen(len);

  if (fclose(fp) != 0) {
    xsyserror("fclose", fname);
  }
}


// EOF
