// flatten.h            see license.txt for copyright and terms of use
// interface to automate process of flattening structures made of objects with
//   arbitrary types, and possibly circular references
// this is a trimmed-down version of the one in 'proot'

// Has a number of similarities with boost::serialize,
//   http://www.boost.org/libs/serialization/doc/index.html
// The main difference is I don't want to use templates.  Also,
// I don't care about STL.

#ifndef SMBASE_FLATTEN_H
#define SMBASE_FLATTEN_H

#include "flatten-fwd.h"               // fwds for this module

#include "trdelete.h"                  // TRASHINGDELETE

#include <stddef.h>                    // size_t
#include <stdint.h>                    // int64_t, int32_t


class Flatten {
public:      // data
  // version of the file being xferred; app must set it if it
  // wants to use it
  int version;

public:      // funcs
  Flatten();
  virtual ~Flatten();

  TRASHINGDELETE;

  // query the read/write state
  virtual bool reading() const = 0;
  bool writing() const { return !reading(); }

  // transferring xfer a simple data type of fixed length
  // 'len', in bytes
  virtual void xferSimple(void *var, size_t len)=0;

  // syntactic sugar
  //#define xferVar(varname) xferSimple(&varname, sizeof(varname))
  //#define XFERV(varname) flat.xferVar(varname)

  // Xfer char and bool as a single octet.
  virtual void xferChar(char &c);
  virtual void xferBool(bool &b);

  // Xfer int with 32 bits and long with 64 bits.  These check that the
  // given values are representable, throwing XFormat if not (both when
  // reading and writing).
  virtual void xferInt32(int &i);
  virtual void xferLong64(long &l);

  // The default implementation of these serializes as 8 octets in big
  // endian order (most significant octet first).
  virtual void xfer_int64_t(int64_t &i);
  virtual void xfer_uint64_t(uint64_t &i);

  // For these, the default implementation serializes as 4 octets, again
  // in big endian order.
  virtual void xfer_int32_t(int32_t &i);
  virtual void xfer_uint32_t(uint32_t &i);

  // read or write a null-terminated character buffer, allocated with
  // new; this works if 'str' is NULL (in other words, a NULL string
  // is distinguished from an empty string, and both are legal)
  virtual void xferCharString(char *&str);

  // xfer a buffer allocated with 'new', of a given length; the buffer
  // may not be NULL (when writing)
  virtual void xferHeapBuffer(void *&buf, size_t len);

  // read: write the code; write: read & compare to code, fail if != ;
  // the code is arbitrary, but should be unique across the source tree
  // (I usually make the code with my Emacs' Ctl-Alt-R, which inserts a random number)
  virtual void checkpoint32(uint32_t code);

  // ------------- utilities ---------
  // for when we already know whether we're reading or writing; internally,
  // these assert which state we're in
  void writeInt32(int i);
  int readInt32();

  // ------------- owner/serf ------------
  // take note of an owner pointer where we expect to xfer serfs to it
  virtual void noteOwner(void *ownerPtr) = 0;

  // xfer a serf pointer that we've previously taken note of
  virtual void xferSerf(void *&serfPtr, bool nullable=false) = 0;
  void writeSerf(void *serfPtr);
  void *readSerf();
};


// Serialize 'intValue' to 'bytes' in network byte order.  The latter
// must be at least 'sizeof(T)' bytes.
template <class T>
void serializeIntNBO(unsigned char *bytes, T intValue)
{
  for (unsigned index=0; index < sizeof(T); index++) {
    bytes[index] = (unsigned char)(intValue >> ((sizeof(T) - 1 - index) * 8));
  }
}


// Deserialize 'bytes' to 'intValue' in network byte order.
template <class T>
void deserializeIntNBO(unsigned char const *bytes, T &intValue)
{
  intValue = 0;
  for (unsigned index=0; index < sizeof(T); index++) {
    intValue |= (T)(bytes[index]) << ((sizeof(T) - 1 - index) * 8);
  }
}


#endif // SMBASE_FLATTEN_H
