// bflatten.h            see license.txt for copyright and terms of use
// binary file flatten implementation

#ifndef BFLATTEN_H
#define BFLATTEN_H

#include "flatten.h"                   // Flatten
#include "ohashtbl.h"                  // OwnerHashTable

#include <iosfwd>                      // std::istream, std::ostream

#include <stdio.h>                     // remove


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
  explicit OwnerTableFlatten(bool reading);
  virtual ~OwnerTableFlatten() override;

  // Flatten funcs
  virtual void noteOwner(void *ownerPtr) override;
  virtual void xferSerf(void *&serfPtr, bool nullable=false) override;
};


// Tagged union of an istream and ostream.
class I_or_OStream {
private:     // data
  // The associated stream.
  union {
    std::istream *m_is;   // active if 'm_readMode'
    std::ostream *m_os;   // active if '!m_readMode'
  } m_stream;

  // True for read mode, false for write mode.
  bool m_readMode;

public:
  explicit I_or_OStream(std::istream *is);
  explicit I_or_OStream(std::ostream *os);

  I_or_OStream(I_or_OStream const &obj);
  I_or_OStream& operator=(I_or_OStream const &obj);

  bool readMode() const { return m_readMode; }

  // These assert that the mode is correct.
  std::istream *is() const;
  std::ostream *os() const;
};


// Serialize to/from a C++ iostream.
class StreamFlatten : public OwnerTableFlatten {
protected:   // data
  // The stream to read or write, and the bool specifying which to do.
  I_or_OStream m_stream;

public:
  explicit StreamFlatten(I_or_OStream stream);
  virtual ~StreamFlatten() override;

  // Flatten funcs
  virtual bool reading() const override { return m_stream.readMode(); }
  virtual void xferSimple(void *var, unsigned len) override;
};


// Serialize to/from a named file.
//
// This class is somewhat poorly named.  The 'B' means "binary", but
// really the entire Flatten API is unsuitable for anything other than
// binary serialization since there are no attribute names.  Clients of
// this class are expecting to read and write a named file, so something
// like NamedFileFlatten would be better, but that means changing a
// handful of clients, which I don't want to do at this moment.
class BFlatten : public StreamFlatten {
public:      // funcs
  // Throws XOpen if cannot open 'fname'.
  BFlatten(char const *fname, bool reading);

  virtual ~BFlatten();
};


// For testing, write and then read something.
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
