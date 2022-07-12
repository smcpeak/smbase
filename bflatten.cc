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

#include "sm-test.h"                   // USUAL_MAIN

#include <sstream>                     // std::i/ostringstream
#include <string>                      // std::string


// Some data members to de/serialize.
class SomeData {
public:      // data
  int x;
  int y;
  string s;
  string s2;
  int *px;
  int *py;
  uint64_t u64;
  int64_t i64;
  uint32_t u32;
  int32_t i32;

public:      // methods
  void init();
  void xfer(Flatten &flat);
  void checkEqual(SomeData const &obj) const;
};


void SomeData::init()
{
  x = 9;
  y = 22;
  s = "foo bar";

  // Test with a string containing both kinds of line endings, in order
  // to verify that no line ending translation is happening.
  s2 = "one\ntwo\r\n";

  px = &x;
  py = &y;
  u64 = (((uint64_t)0x12345678) << 32) | 0x90ABCDEF;
  i64 = -((int64_t)u64);
  u32 = 0x21436587;
  i32 = -((int32_t)u32);
}


void SomeData::xfer(Flatten &flat)
{
  flat.xferInt(x);
  flat.noteOwner(&x);
  s.xfer(flat);
  s2.xfer(flat);
  flat.xferSerf((void*&)px);
  flat.xferInt(y);
  flat.noteOwner(&y);
  flat.xferSerf((void*&)py);
  flat.xfer_uint64_t(u64);
  flat.xfer_int64_t(i64);
  flat.xfer_uint32_t(u32);
  flat.xfer_int32_t(i32);
}


void SomeData::checkEqual(SomeData const &obj) const
{
  xassert(EMEMB(x));
  xassert(EMEMB(y));
  xassert(EMEMB(s));
  xassert(EMEMB(s2));
  xassert(EMEMB(u64));
  xassert(EMEMB(i64));
  xassert(EMEMB(u32));
  xassert(EMEMB(i32));

  // This does not compare to 'obj', rather it checks a condition that I
  // know 'init' created in 'obj', and should be re-created by
  // deserialization.
  xassert(px == &x);
  xassert(py == &y);
}


void entry()
{
  // make up some data
  SomeData d1;
  d1.init();

  // open a file for writing them
  {
    BFlatten flat("bflat.tmp", false /*reading*/);
    d1.xfer(flat);
  }

  // Also save to an in-memory string.
  std::string serializedString;
  {
    std::ostringstream oss;
    StreamFlatten flat(&oss);
    d1.xfer(flat);
    serializedString = oss.str();
  }

  // place to put the data we read
  SomeData d2;

  // read them back
  {
    BFlatten flat("bflat.tmp", true /*reading*/);
    d2.xfer(flat);
  }

  // compare
  d2.checkEqual(d1);

  // delete the temp file
  remove("bflat.tmp");

  // Deserialize the string.
  SomeData d3;
  {
    std::istringstream iss(serializedString);
    StreamFlatten flat(&iss);
    d3.xfer(flat);
  }
  d3.checkEqual(d1);

  printf("bflatten works\n");
}


USUAL_MAIN


#endif // TEST_BFLATTEN
