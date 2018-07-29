// flatten.h            see license.txt for copyright and terms of use
// interface to automate process of flattening structures made of objects with
//   arbitrary types, and possibly circular references
// this is a trimmed-down version of the one in 'proot'

// Has a number of similarities with boost::serialize,
//   http://www.boost.org/libs/serialization/doc/index.html
// The main difference is I don't want to use templates.  Also,
// I don't care about STL.

#ifndef FLATTEN_H
#define FLATTEN_H

#include "trdelete.h"   // TRASHINGDELETE

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
  virtual void xferSimple(void *var, unsigned len)=0;

  // syntactic sugar
  //#define xferVar(varname) xferSimple(&varname, sizeof(varname))
  //#define XFERV(varname) flat.xferVar(varname)

  // xfer various C built-in data types (will add them as I need them)
  virtual void xferChar(char &c);
  virtual void xferInt(int &i);
  virtual void xferLong(long &l);
  virtual void xferBool(bool &b);

  // read or write a null-terminated character buffer, allocated with
  // new; this works if 'str' is NULL (in other words, a NULL string
  // is distinguished from an empty string, and both are legal)
  virtual void xferCharString(char *&str);

  // xfer a buffer allocated with 'new', of a given length; the buffer
  // may not be NULL (when writing), and len must be nonnegative
  virtual void xferHeapBuffer(void *&buf, int len);

  // read: write the code; write: read & compare to code, fail if != ;
  // the code is arbitrary, but should be unique across the source tree
  // (I usually make the code with my Emacs' Ctl-Alt-R, which inserts a random number)
  virtual void checkpoint(int code);

  // ------------- utilities ---------
  // for when we already know whether we're reading or writing; internally,
  // these assert which state we're in
  void writeInt(int i);
  int readInt();

  // ------------- owner/serf ------------
  // take note of an owner pointer where we expect to xfer serfs to it
  virtual void noteOwner(void *ownerPtr) = 0;

  // xfer a serf pointer that we've previously taken note of
  virtual void xferSerf(void *&serfPtr, bool nullable=false) = 0;
  void writeSerf(void *serfPtr);
  void *readSerf();
};

#endif // FLATTEN_H
