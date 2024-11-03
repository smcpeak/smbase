// bflatten.cc
// Code for bflatten.h.

// See license.txt for copyright and terms of use.

#include "bflatten.h"                  // this module

#include "smbase/exc.h"                // xformat, GENERIC_CATCH_{BEGIN,END}
#include "smbase/hashtbl.h"            // HashTable
#include "smbase/overflow.h"           // convertNumber
#include "smbase/sm-macros.h"          // STATICDEF
#include "smbase/sm-stdint.h"          // intptr_t
#include "smbase/stringb.h"            // stringb
#include "smbase/syserr.h"             // xsyserror
#include "smbase/xassert.h"            // xassert

#include <fstream>                     // std::fstream, etc.

using namespace smbase;


// ----------------------- OwnerTableFlatten ---------------------------
OwnerTableFlatten::OwnerTableFlatten(bool reading)
  : ownerTable(!reading? &BFlatten::getOwnerPtrKeyFn : &BFlatten::getIntNameKeyFn,
               HashTable::lcprngHashFn,
               HashTable::pointerEqualKeyFn),
    nextUniqueName(1)
{}


OwnerTableFlatten::~OwnerTableFlatten()
{}


STATICDEF void const* OwnerTableFlatten::getOwnerPtrKeyFn(OwnerMapping *data)
{
  return data->ownerPtr;
}

STATICDEF void const* OwnerTableFlatten::getIntNameKeyFn(OwnerMapping *data)
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
      writeInt32(0);
    }
    else {
      // lookup the mapping
      OwnerMapping *map = ownerTable.get(serfPtr);

      // we must have already written the owner pointer
      xassert(map != NULL);

      // write the int name
      writeInt32(map->intName);
    }
  }
  else /*reading*/ {
    // read the int name
    int name = readInt32();

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


void StreamFlatten::xferSimple(void *var, size_t len)
{
  if (writing()) {
    // Ensure there cannot be an overflow when passing 'len' as the
    // 'count' argument to 'write'.
    static_assert(sizeof(size_t) <= sizeof(std::streamsize), "");

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
    if (convertNumber<size_t>(ct) < len) {
      xformat(stringb("unexpected end of input (ct=" << ct <<
                      ", len=" << len << ")"));
    }
  }
}


// ---------------------------- BFlatten -------------------------------
// Attempt to open an ifstream or ofstream for 'fname', throwing
// XSysError if the attempt fails.
template <class T>
T *tryOpen(char const *fname)
{
  T *stream = new T(fname, std::ios::binary);
  if (stream->fail()) {
    delete stream;
    xsyserror("open", fname);
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
  GENERIC_CATCH_BEGIN

  if (reading()) {
    delete m_stream.is();
  }
  else {
    delete m_stream.os();
  }

  GENERIC_CATCH_END
}


// EOF
