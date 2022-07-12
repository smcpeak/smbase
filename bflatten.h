// bflatten.h            see license.txt for copyright and terms of use
// binary file flatten implementation

#ifndef BFLATTEN_H
#define BFLATTEN_H

#include "flatten.h"      // Flatten
#include "ohashtbl.h"     // OwnerHashTable
#include <stdio.h>        // FILE


// Partial Flatten implementation that handles serialization of owner
// and serf pointers.
class OwnerTableFlatten : public Flatten {
private:     // data
  struct OwnerMapping {
    void *ownerPtr;       // a pointer
    int intName;          // a unique integer name
  };
  OwnerHashTable<OwnerMapping> ownerTable;      // owner <-> int mapping
  int nextUniqueName;     // counter for making int names

private:     // methods
  static void const* getOwnerPtrKeyFn(OwnerMapping *data);
  static void const* getIntNameKeyFn(OwnerMapping *data);

public:      // methods
  OwnerTableFlatten(bool reading);
  virtual ~OwnerTableFlatten() override;

  // Flatten funcs
  virtual void noteOwner(void *ownerPtr) override;
  virtual void xferSerf(void *&serfPtr, bool nullable=false) override;
};


class BFlatten : public OwnerTableFlatten {
private:     // data
  FILE *fp;               // file being read/written
  bool readMode;          // true=read, false=write

public:      // funcs
  // throws XOpen if cannot open 'fname'
  BFlatten(char const *fname, bool reading);
  virtual ~BFlatten();

  // Flatten funcs
  virtual bool reading() const override { return readMode; }
  virtual void xferSimple(void *var, unsigned len) override;
};


// for debugging, write and then read something
template <class T>
T *writeThenRead(T &obj)
{
  char const *fname = "flattest.tmp";

  // write
  {
    BFlatten out(fname, false /*reading*/);
    obj.xfer(out);
  }

  // read
  BFlatten in(fname, true /*reading*/);
  T *ret = new T(in);
  ret->xfer(in);

  remove(fname);

  return ret;
}

#endif // BFLATTEN_H
