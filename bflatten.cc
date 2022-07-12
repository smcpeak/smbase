// bflatten.cc            see license.txt for copyright and terms of use
// code for bflatten.h

#include "bflatten.h"     // this module
#include "exc.h"          // throw_XOpen, xformat
#include "sm-stdint.h"    // intptr_t
#include "syserr.h"       // xsyserror

#include <fstream>        // std::fstream, etc.


// ----------------------- OwnerTableFlatten ---------------------------
OwnerTableFlatten::OwnerTableFlatten(bool reading)
  : ownerTable(!reading? &BFlatten::getOwnerPtrKeyFn : &BFlatten::getIntNameKeyFn,
               HashTable::lcprngHashFn,
               HashTable::pointerEqualKeyFn),
    nextUniqueName(1)
{}


OwnerTableFlatten::~OwnerTableFlatten()
{}


/*static*/ void const* OwnerTableFlatten::getOwnerPtrKeyFn(OwnerMapping *data)
{
  return data->ownerPtr;
}

/*static*/ void const* OwnerTableFlatten::getIntNameKeyFn(OwnerMapping *data)
{
  return (void const*)(intptr_t)(data->intName);
}


void OwnerTableFlatten::noteOwner(void *ownerPtr)
{
  // make a new mapping
  OwnerMapping *map = new OwnerMapping;
  map->ownerPtr = ownerPtr;
  map->intName = nextUniqueName++;

  // add it to the table
  if (writing()) {
    // index by pointer
    ownerTable.add(ownerPtr, map);
  }
  else {
    // index by int name
    ownerTable.add((void const*)(intptr_t)(map->intName), map);
  }
}


void OwnerTableFlatten::xferSerf(void *&serfPtr, bool isNullable)
{
  if (writing()) {
    xassert(isNullable || serfPtr!=NULL);

    if (serfPtr == NULL) {
      // encode as 0; the names start with 1
      writeInt(0);
    }
    else {
      // lookup the mapping
      OwnerMapping *map = ownerTable.get(serfPtr);

      // we must have already written the owner pointer
      xassert(map != NULL);

      // write the int name
      writeInt(map->intName);
    }
  }
  else /*reading*/ {
    // read the int name
    int name = readInt();

    if (name == 0) {      // null
      xassert(isNullable);
      serfPtr = NULL;
    }
    else {
      // lookup the mapping
      OwnerMapping *map = ownerTable.get((void const*)(intptr_t)name);
      formatAssert(map != NULL);

      // return the pointer
      serfPtr = map->ownerPtr;
    }
  }
}


// --------------------------- I_or_OStream ----------------------------
I_or_OStream::I_or_OStream(std::istream *is)
  : m_readMode(true)
{
  m_stream.m_is = is;
}


I_or_OStream::I_or_OStream(std::ostream *os)
  : m_readMode(false)
{
  m_stream.m_os = os;
}


I_or_OStream::I_or_OStream(I_or_OStream const &obj)
{
  *this = obj;
}


I_or_OStream& I_or_OStream::operator=(I_or_OStream const &obj)
{
  m_readMode = obj.m_readMode;
  if (m_readMode) {
    m_stream.m_is = obj.m_stream.m_is;
  }
  else {
    m_stream.m_os = obj.m_stream.m_os;
  }

  return *this;
}


std::istream *I_or_OStream::is() const
{
  xassert(m_readMode);
  return m_stream.m_is;
}


std::ostream *I_or_OStream::os() const
{
  xassert(!m_readMode);
  return m_stream.m_os;
}


// -------------------------- StreamFlatten ----------------------------
StreamFlatten::StreamFlatten(I_or_OStream stream)
  : OwnerTableFlatten(stream.readMode()),
    m_stream(stream)
{}


StreamFlatten::~StreamFlatten()
{}


void StreamFlatten::xferSimple(void *var, unsigned len)
{
  if (writing()) {
    m_stream.os()->write((char const*)var, len);
    if (!m_stream.os()->good()) {
      xsyserror("write");
    }
  }
  else {
    m_stream.is()->read((char*)var, len);
    if (!m_stream.is()->good()) {
      xsyserror("read");
    }
    std::streamsize ct = m_stream.is()->gcount();
    if (ct < len) {
      xformat(stringb("unexpected end of input (ct=" << ct <<
                      ", len=" << len << ")"));
    }
  }
}


// ---------------------------- BFlatten -------------------------------
// Attempt to open an ifstream or ofstream for 'fname', throwing XOpen
// if the attempt fails.
template <class T>
T *tryOpen(char const *fname)
{
  T *stream = new T(fname, std::ios::binary);
  if (stream->fail()) {
    delete stream;
    throw_XOpen(fname);
  }
  return stream;
}


BFlatten::BFlatten(char const *fname, bool r)
  : StreamFlatten(r?
      I_or_OStream(tryOpen<std::ifstream>(fname)) :
      I_or_OStream(tryOpen<std::ofstream>(fname)))
{}


BFlatten::~BFlatten()
{
  if (reading()) {
    delete m_stream.is();
  }
  else {
    delete m_stream.os();
  }
}


// ------------------------ test code ---------------------
#ifdef TEST_BFLATTEN

#include "sm-test.h"   // USUAL_MAIN

void entry()
{
  // make up some data
  int x = 9, y = 22;
  string s("foo bar");
  int *px = &x, *py = &y;
  uint64_t u64 = (((uint64_t)0x12345678) << 32) | 0x90ABCDEF;
  int64_t i64 = -((int64_t)u64);
  uint32_t u32 = 0x21436587;
  int32_t i32 = -((int32_t)u32);

  // open a file for writing them
  {
    BFlatten flat("bflat.tmp", false /*reading*/);
    flat.xferInt(x);
    flat.noteOwner(&x);
    s.xfer(flat);
    flat.xferSerf((void*&)px);
    flat.xferInt(y);
    flat.noteOwner(&y);
    flat.xferSerf((void*&)py);
    flat.xfer_uint64_t(u64);
    flat.xfer_int64_t(i64);
    flat.xfer_uint32_t(u32);
    flat.xfer_int32_t(i32);
  }

  // place to put the data we read
  int x2, y2;
  string s2;
  int *px2, *py2;
  uint64_t u64b;
  int64_t i64b;
  uint32_t u32b;
  int32_t i32b;

  // read them back
  {
    BFlatten flat("bflat.tmp", true /*reading*/);
    flat.xferInt(x2);
    flat.noteOwner(&x2);
    s2.xfer(flat);
    flat.xferSerf((void*&)px2);
    flat.xferInt(y2);
    flat.noteOwner(&y2);
    flat.xferSerf((void*&)py2);
    flat.xfer_uint64_t(u64b);
    flat.xfer_int64_t(i64b);
    flat.xfer_uint32_t(u32b);
    flat.xfer_int32_t(i32b);
  }

  // compare
  xassert(x == x2);
  xassert(y == y2);
  xassert(s.equals(s2));
  xassert(px2 == &x2);
  xassert(py2 == &y2);
  xassert(u64 == u64b);
  xassert(i64 == i64b);
  xassert(u32 == u32b);
  xassert(i32 == i32b);

  // delete the temp file
  remove("bflat.tmp");

  printf("bflatten works\n");
}


USUAL_MAIN


#endif // TEST_BFLATTEN
