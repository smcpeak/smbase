// flatutil.h
// additional utilities layered upon 'flatten' interface

#ifndef FLATUTIL_H
#define FLATUTIL_H

#include "flatten.h"                   // underlying module
#include "objlist.h"                   // ObjList
#include "overflow.h"                  // convertWithoutLoss
#include "xassert.h"                   // xassert

#include <vector>                      // std::vector


// Nominal way to create a 'T' object for unflattening; but you
// can overload to do things differently where needed.
template <class T>
T *createForUnflat(Flatten &flat)
{
  return new T(flat);
}

// Nominal way to flatten.  Again, overload to override.
template <class T>
void xfer(Flatten &flat, T &t)
{
  t.xfer(flat);
}


// Transfer an owner list.  First we transfer the number of elements,
// then each element in sequence.  If 'noteOwner' is true, the
// pointers are noted so that it is possible to later transfer serf
// aliases.
template <class T>
void xferObjList(Flatten &flat, ObjList<T> &list, bool noteOwner = false)
{
  if (flat.writing()) {
    flat.writeInt32(list.count());
    FOREACH_OBJLIST_NC(T, list, iter) {
      T *t = iter.data();
      xfer(flat, *t);
      if (noteOwner) {
        flat.noteOwner(t);
      }
    }
  }
  else {
    list.deleteAll();
    int ct = flat.readInt32();
    while (ct--) {
      T *t = createForUnflat<T>(flat);
      xfer(flat, *t);
      if (noteOwner) {
        flat.noteOwner(t);
      }
      list.prepend(t);
    }
    list.reverse();
  }
}


// 2022-07-12: I removed 'value_cast'.  Its replacement is
// 'convertWithoutLoss', defined in overflow.h.


// Transfer an enum value.  This is safer than just casting to int
// reference, since it works when 'int' is not the same size as the
// enum.
template <class E>
void xferEnum(Flatten &flat, E &e)
{
  int32_t i = 0;
  if (flat.writing()) {
    convertWithoutLoss(i, e);
    flat.xfer_int32_t(i);
  }
  else {
    flat.xfer_int32_t(i);
    convertWithoutLoss(e, i);
  }
}


// xfer a std::vector bytewise.
//
// It is not good to do this if T contains any scalar value that is
// larger than a byte due to the resulting dependence on endianness.
template <class T>
void xferVectorBytewise(Flatten &flat, std::vector<T> &vec)
{
  // Read or write length.
  {
    int64_t numElements = 0;

    if (flat.writing()) {
      // Write length in elements.
      convertWithoutLoss(numElements, vec.size());
      flat.xfer_int64_t(numElements);
    }

    else {
      // Read length in elements.
      flat.xfer_int64_t(numElements);

      // Convert to size_t with overflow check.
      size_t st_ne;
      convertWithoutLoss(st_ne, numElements);

      // Set vector size accordingly.
      vec.resize(st_ne);
    }
  }

  // Read or write data.
  size_t numBytes = multiplyWithOverflowCheck<size_t>(vec.size(), sizeof(T));
  flat.xferSimple(vec.data(), numBytes);
}


#endif // FLATUTIL_H
