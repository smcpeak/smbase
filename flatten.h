// flatten.h
// interface to automate process of flattening structures made of objects with
//   arbitrary types, and possibly circular references
// this is a trimmed-down version of the one in 'proot'

#ifndef FLATTEN_H
#define FLATTEN_H

#include "trdelete.h"   // TRASHINGDELETE

class Flatten {
public:
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

  // read or write a null-terminated character buffer, allocated with new
  virtual void xferCharString(char *&str);

  // read: write the code; write: read & compare to code, fail if != ;
  // the code is arbitrary, but should be unique across the source tree
  virtual void checkpoint(int code);
  
  // ------------- utilities ---------
  // for when we already know whether we're reading or writing; internally,
  // these assert which state we're in
  void writeInt(int i);
  int readInt(); 

};

#endif // FLATTEN_H
