// array.h            see license.txt for copyright and terms of use
// some array classes

#ifndef ARRAY_H
#define ARRAY_H

#include "sm-macros.h"                 // NO_OBJECT_COPIES
#include "sm-swap.h"                   // swap
#include "str.h"                       // string
#include "xassert.h"                   // xassert

#include <stdlib.h>                    // qsort


// -------------------- Array ----------------------
// This is the same as C++'s built-in array, but automatically deallocates.
// If you want bounds checking too, use GrowArray, below.
template <class T>
class Array {
private:     // data
  T *arr;

private:     // not allowed
  Array(Array&);
  void operator=(Array&);

public:
  explicit Array(int len)
    : arr(new T[(len>=0? len :
                  (xfailure("Array with negative length"), 0) )])
  {}
  ~Array() { delete[] arr; }

  T const &operator[] (int i) const { return arr[i]; }
  T &operator[] (int i) { return arr[i]; }

  T const *ptrC() const { return arr; }
  T *ptr() { return arr; }

  // convenience
  void setAll(T val, int len) {
    for (int i=0; i<len; i++) {
      arr[i] = val;
    }
  }
};


// ------------------ GrowArray --------------------
// This class implements an array of T's; it automatically expands
// when 'ensureAtLeast' or 'ensureIndexDoubler' is used; it does not
// automatically contract.  All accesses are bounds-checked.
//
// class T must have:
//   T::T();           // default ctor for making arrays
//   operator=(T&);    // assignment for copying to new storage
//   T::~T();          // dtor for when old array is cleared
template <class T>
class GrowArray {
private:     // data
  T *arr;                 // underlying array; NULL if sz==0
  int sz;                 // # allocated entries in 'arr'

private:     // funcs
  void bc(int i) const    // bounds-check an index
    { xassert((unsigned)i < (unsigned)sz); }
  void eidLoop(int index);

  // make 'this' equal to 'obj'
  void copyFrom(GrowArray<T> const &obj) {
    setAllocatedSize(obj.allocatedSize()); // not terribly efficient, oh well
    copyFrom_limit(obj, sz);
  }

protected:   // funcs
  void copyFrom_limit(GrowArray<T> const &obj, int limit);

private:     // disallowed
  void operator=(GrowArray&);
  void operator==(GrowArray&);

public:      // funcs
  explicit GrowArray(int initSz);
  GrowArray(GrowArray const &obj) : arr(0), sz(0) { copyFrom(obj); }
  ~GrowArray();

  GrowArray& operator=(GrowArray const &obj) { copyFrom(obj); return *this; }

  // Allocated space, as number of elements in the array.
  int allocatedSize() const { return sz; }

  // element access
  T const& operator[] (int i) const   { bc(i); return arr[i]; }
  T      & operator[] (int i)         { bc(i); return arr[i]; }

  // set size, reallocating if old size is different; if the
  // array gets bigger, existing elements are preserved; if the
  // array gets smaller, elements are truncated
  void setAllocatedSize(int newSz);

  // make sure there are at least 'minSz' elements in the array;
  void ensureAtLeast(int minSz)
    { if (minSz > sz) { setAllocatedSize(minSz); } }

  // grab a read-only pointer to the raw array
  T const *getArray() const { return arr; }

  // grab a writable pointer; use with care
  T *getDangerousWritableArray() { return arr; }
  T *getArrayNC() { return arr; }     // ok, not all that dangerous..

  // make sure the given index is valid; if this requires growing,
  // do so by doubling the size of the array (repeatedly, if
  // necessary)
  void ensureIndexDoubler(int index)
    { if (sz-1 < index) { eidLoop(index); } }

  // set an element, using the doubler if necessary
  void setIndexDoubler(int index, T const &value)
    { ensureIndexDoubler(index); arr[index] = value; }

  // swap my data with the data in another GrowArray object
  void swapWith(GrowArray<T> &obj) {
    T *tmp1 = obj.arr; obj.arr = this->arr; this->arr = tmp1;
    int tmp2 = obj.sz; obj.sz = this->sz; this->sz = tmp2;
  }

  // set all elements to a single value
  void setAll(T val) {
    for (int i=0; i<sz; i++) {
      arr[i] = val;
    }
  }

  // Move the item at 'oldIndex' so it occupies 'newIndex' instead,
  // shifting the intervening elements by one spot.  Both arguments
  // must be in [0,allocatedSize()-1].
  void moveElement(int oldIndex, int newIndex);
};


template <class T>
GrowArray<T>::GrowArray(int initSz)
{
  sz = initSz;
  if (sz > 0) {
    arr = new T[sz];
  }
  else {
    arr = NULL;
  }
}


template <class T>
GrowArray<T>::~GrowArray()
{
  if (arr) {
    delete[] arr;
  }
}


template <class T>
void GrowArray<T>::copyFrom_limit(GrowArray<T> const &obj, int limit)
{
  for (int i=0; i<limit; i++) {
    arr[i] = obj.arr[i];
  }
}


template <class T>
void GrowArray<T>::setAllocatedSize(int newSz)
{
  if (newSz != sz) {
    // keep track of old
    int oldSz = sz;
    T *oldArr = arr;

    // make new
    sz = newSz;
    if (sz > 0) {
      arr = new T[sz];
    }
    else {
      arr = NULL;
    }

    // copy elements in common
    for (int i=0; i<sz && i<oldSz; i++) {
      arr[i] = oldArr[i];
    }

    // get rid of old
    if (oldArr) {
      delete[] oldArr;
    }
  }
}


// this used to be ensureIndexDoubler's implementation, but
// I wanted the very first check to be inlined
template <class T>
void GrowArray<T>::eidLoop(int index)
{
  if (sz-1 >= index) {
    return;
  }

  int newSz = sz;
  while (newSz-1 < index) {
    #ifndef NDEBUG_NO_ASSERTIONS    // silence warning..
      int prevSz = newSz;
    #endif
    if (newSz == 0) {
      newSz = 1;
    }
    newSz = newSz*2;
    xassert(newSz > prevSz);        // otherwise overflow -> infinite loop
  }

  setAllocatedSize(newSz);
}


template <class T>
void GrowArray<T>::moveElement(int oldIndex, int newIndex)
{
  this->bc(oldIndex);
  this->bc(newIndex);

  while (oldIndex < newIndex) {
    swap(this->arr[oldIndex], this->arr[oldIndex+1]);
    oldIndex++;
  }

  while (oldIndex > newIndex) {
    swap(this->arr[oldIndex], this->arr[oldIndex-1]);
    oldIndex--;
  }
}


// ---------------------- ArrayStack ---------------------
// This is an array where some of the array is unused.  Specifically,
// it maintains a 'length', and elements 0 up to length-1 are
// considered used, whereas length up to size-1 are unused.  The
// expected use is as a stack, where "push" adds a new (used) element.
template <class T>
class ArrayStack : public GrowArray<T> {
private:     // data
  int len;               // # of elts in the stack

private:     // funcs
  void bc(int i) const { xassert((unsigned)i < (unsigned)len); }

public:      // funcs
  explicit ArrayStack(int initArraySize = 0)
    : GrowArray<T>(initArraySize),
      len(0)
    {}
  ArrayStack(ArrayStack<T> const &obj)
    : GrowArray<T>(obj),
      len(obj.len)
    {}
  ~ArrayStack();

  // copies contents of 'obj', but the allocated size of 'this' will
  // only change when necessary
  ArrayStack& operator=(ArrayStack<T> const &obj)
  {
    this->ensureIndexDoubler(obj.length() - 1);
    this->copyFrom_limit(obj, obj.length());
    len = obj.len;
    return *this;
  }

  // element access; these declarations are necessary because
  // the uses of 'operator[]' below are not "dependent", hence
  // they can't use declarations inherited from GrowArray<T>
  T const& operator[] (int i) const { return GrowArray<T>::operator[](i); }
  T      & operator[] (int i)       { return GrowArray<T>::operator[](i); }

  void push(T const &val)
    { this->setIndexDoubler(len++, val); }
  T pop()
    { return operator[](--len); }
  T const &top() const
    { return operator[](len-1); }
  T &top()
    { return operator[](len-1); }
  T &nth(int which)
    { return operator[](len-1-which); }

  // alternate interface, where init/deinit is done explicitly
  // on returned references
  T &pushAlt()    // returns newly accessible item
    { GrowArray<T>::ensureIndexDoubler(len++); return top(); }
  T &popAlt()     // returns item popped
    { return operator[](--len); }

  // Push a block of 'numToPush' uninitialized elements and return a
  // pointer to the first one.  The intended use is to immediately write
  // into that pointer to set the value of these additional elements.
  // The new objects are uninitialized in that they could be newly added
  // and hence default-constructed but could also be left over from some
  // prior use.
  T *ptrToPushedMultipleAlt(int numToPush);

  // items stored
  int length() const
    { return len; }

  bool isEmpty() const
    { return len==0; }
  bool isNotEmpty() const
    { return !isEmpty(); }

  // Return index of element 't' or -1 if not in the array.
  int indexOf(T const &t) const;

  void popMany(int ct)
    { len -= ct; xassert(len >= 0); }
  void empty()        // TODO: Rename this!  STL collision.
    { len = 0; }
  void clear()
    { len = 0; }

  // useful when someone has used 'getDangerousWritableArray' to
  // fill the array's internal storage
  void setLength(int L) { len = L; }

  // consolidate allocated space to match length
  void consolidate() { this->setAllocatedSize(length()); }

  // swap
  void swapWith(ArrayStack<T> &obj) {
    GrowArray<T>::swapWith(obj);
    int tmp = obj.len; obj.len = this->len; this->len = tmp;
  }

  void sort(int (*compare)(T const *t1, T const *t2)) {
    qsort(GrowArray<T>::getArrayNC(), len, sizeof(T),
          (int (*)(void const*, void const*))compare );
  }

  // Move the item at 'oldIndex' so it occupies 'newIndex' instead,
  // shifting the intervening elements by one spot.  Both arguments
  // must be in [0,length()-1].
  void moveElement(int oldIndex, int newIndex);
};

template <class T>
ArrayStack<T>::~ArrayStack()
{}


template <class T>
T *ArrayStack<T>::ptrToPushedMultipleAlt(int numToPush)
{
  // 'ensureIndexDoubler' is slightly awkward as it wants the maximum
  // valid index rather than the length.
  int oldLength = this->length();
  this->ensureIndexDoubler(oldLength + numToPush - 1);

  // Bump the length to include these new elements.
  this->setLength(oldLength + numToPush);

  // Return a pointer to the new area.
  return this->getArrayNC() + oldLength;
}


template <class T>
int ArrayStack<T>::indexOf(T const &t) const
{
  for (int i=0; i < this->length(); i++) {
    if (this->operator[](i) == t) {
      return i;
    }
  }
  return -1;
}


template <class T>
void ArrayStack<T>::moveElement(int oldIndex, int newIndex)
{
  // GrowArray also checks bounds, but only against the allocated
  // size, not the ArrayStack 'len' field.
  this->bc(oldIndex);
  this->bc(newIndex);

  this->GrowArray<T>::moveElement(oldIndex, newIndex);
}


// Compare two ArrayStacks elementwise for equality.
template <class T>
bool operator== (ArrayStack<T> const &a1, ArrayStack<T> const &a2)
{
  if (a1.length() != a2.length()) {
    return false;
  }
  for (int i=0; i < a1.length(); i++) {
    if (a1[i] != a2[i]) {
      return false;
    }
  }
  return true;
}

template <class T>
bool operator!= (ArrayStack<T> const &a1, ArrayStack<T> const &a2)
{
  return !operator==(a1, a2);
}


inline string toString(ArrayStack<char> const &arr)
{
  return string(arr.getArray(), arr.length());
}


inline string toString(ArrayStack<unsigned char> const &arr)
{
  return string((char const *)arr.getArray(), arr.length());
}


// iterator over contents of an ArrayStack, to make it easier to
// switch between it and SObjList as a representation
template <class T>
class ArrayStackIterNC {
  NO_OBJECT_COPIES(ArrayStackIterNC);   // for now

private:     // data
  ArrayStack<T> /*const*/ &arr;   // array being accessed
  int index;                      // current element

public:      // funcs
  explicit ArrayStackIterNC(ArrayStack<T> /*const*/ &a)
    : arr(a), index(0) {}

  // iterator actions
  bool isDone() const             { return index >= arr.length(); }
  void adv()                      { xassert(!isDone()); index++; }
  T /*const*/ *data() const       { return &(arr[index]); }
};

#define FOREACH_ARRAYSTACK_NC(T, list, iter) \
  for(ArrayStackIterNC< T > iter(list); !iter.isDone(); iter.adv())


// I want const polymorphism!


// pop (and discard) a value off a stack at end of scope
template <class T>
class ArrayStackPopper {
private:
  ArrayStack<T> &stk;

public:
  explicit ArrayStackPopper(ArrayStack<T> &s) : stk(s) {}
  ArrayStackPopper(ArrayStack<T> &s, T const &pushVal)
    : stk(s) { stk.push(pushVal); }
  ~ArrayStackPopper()
    { stk.pop(); }
};

template <class T> class ObjArrayStackIterNC;


// Remove all elements from 'arr' for which 'condition' is false.
template <class T, class FUNCTYPE>
void applyFilter(ArrayStack<T> &arr, FUNCTYPE condition)
{
  // Where to place the next element that satisfies the condition.
  int destIndex = 0;

  // Next element to test.
  int srcIndex = 0;

  while (srcIndex < arr.length()) {
    if (condition(arr[srcIndex])) {
      // Keep the element.
      if (destIndex != srcIndex) {
        swap(arr[destIndex], arr[srcIndex]);
      }
      destIndex++;
      srcIndex++;
    }
    else {
      // Discard the element.
      srcIndex++;
    }
  }

  // Trim the array, removing all elements at or after 'destIndex'.
  arr.popMany(srcIndex - destIndex);
}


// ------------------- ObjArrayStack -----------------
// an ArrayStack of owner pointers
template <class T>
class ObjArrayStack {
  NO_OBJECT_COPIES(ObjArrayStack);
  friend class ObjArrayStackIterNC<T>;

private:    // data
  ArrayStack<T*> arr;

public:     // funcs
  explicit ObjArrayStack(int initArraySize = 0)
    : arr(initArraySize)
    {}
  ~ObjArrayStack() { deleteAll(); }

  void push(T *ptr)          { arr.push(ptr); }
  // synonym of 'push', for compatibility with ObjList
  void append(T *ptr)        { arr.push(ptr); }
  T *pop()                   { return arr.pop(); }

  T const *topC() const      { return arr.top(); }
  T       *top()             { return arr.top(); }

  T const * operator[](int index) const  { return arr[index]; }
  T *       operator[](int index)        { return arr[index]; }

  // Return index of element 't' or -1 if not in the array.
  int indexOf(T const *t) const;

  // Replace the element at 'index' with 'newPtr', returning the old
  // element pointer (as an owner pointer).
  T *swapAt(int index, T *newPtr)
  {
    T *ret = arr[index];
    arr[index] = newPtr;
    return ret;
  }

  int length() const         { return arr.length(); }
  bool isEmpty() const       { return arr.isEmpty(); }
  bool isNotEmpty() const    { return !isEmpty(); }

  int allocatedSize() const  { return arr.allocatedSize(); }

  void deleteTopSeveral(int ct);
  void deleteAll()           { deleteTopSeveral(length()); }

  // remove an element from the middle, shifting others down to
  // maintain the original order
  T *removeIntermediate(int toRemove);

  // will not delete any items
  void consolidate()         { arr.consolidate(); }

  void swapWith(ObjArrayStack<T> &obj)
    { arr.swapWith(obj.arr); }

  void moveElement(int oldIndex, int newIndex)
    { arr.moveElement(oldIndex, newIndex); }
};


template <class T>
int ObjArrayStack<T>::indexOf(T const *t) const
{
  // The const_cast here is justified by the fact that the inner
  // 'indexOf' will only use the pointer value since it does not
  // even know it is a pointer, and it will not store that value.
  // It is required simply because 'arr' is an ArrayStack<T*>.
  return arr.indexOf(const_cast<T*>(t));
}


template <class T>
void ObjArrayStack<T>::deleteTopSeveral(int ct)
{
  while (ct--) {
    delete pop();
  }
}


template <class T>
T *ObjArrayStack<T>::removeIntermediate(int toRemove)
{
  T *ret = arr[toRemove];

  // shift remaining elements down
  for (int i=toRemove+1; i < length(); i++) {
    arr[i-1] = arr[i];
  }

  // remove and throw away the final (now redundant) pointer
  pop();

  return ret;
}

template <class T>
class ObjArrayStackIterNC {
  NO_OBJECT_COPIES(ObjArrayStackIterNC);

private:     // data
  ObjArrayStack<T> /*const*/ &arr;   // array being accessed
  int index;                      // current element

public:      // funcs
  explicit ObjArrayStackIterNC(ObjArrayStack<T> /*const*/ &a)
    : arr(a), index(0) {}

  // iterator actions
  bool isDone() const             { return index >= arr.length(); }
  void adv()                      { xassert(!isDone()); index++; }
  T /*const*/ *data() const       { return arr[index]; }
};

#define FOREACH_OBJARRAYSTACK_NC(T, list, iter) \
  for(ObjArrayStackIterNC< T > iter(list); !iter.isDone(); iter.adv())

// ------------------------- ArrayStackEmbed --------------------------
// This is like ArrayStack, but the first 'n' elements are stored
// embedded in this object, instead of allocated on the heap; in some
// circumstances, this lets us avoid allocating memory in common cases.
//
// For example, suppose you have an algorithm that is usually given a
// small number of elements, say 1 or 2, but occasionally needs to
// work with more.  If you put the array of elements in the heap, then
// even in the common case a heap allocation is required, which is
// bad.  But by using ArrayStackEmbed<T,2>, you can be sure that if
// the number of elements is <= 2 there will be no heap allocation,
// even though you still get a uniform (array-like) interface to all
// the elements.
template <class T, int n>
class ArrayStackEmbed {
private:      // data
  // embedded storage
  T embed[n];

  // heap-allocated storage
  GrowArray<T> heap;

  // total number of elements in the stack; if this
  // exceeds 'n', then heap.arr is non-NULL
  int len;

private:      // funcs
  void bc(int i) const    // bounds-check an index
    { xassert((unsigned)i < (unsigned)len); }

public:       // funcs
  ArrayStackEmbed()
    : /*embed is default-init'd*/
      heap(0),    // initially a NULL ptr
      len(0)
  {}
  ~ArrayStackEmbed()
  {}              // heap auto-deallocs its internal data

  void push(T const &val)
  {
    if (len < n) {
      embed[len++] = val;
    }
    else {
      heap.setIndexDoubler(len++ - n, val);
    }
  }

  T pop()
  {
    xassert(len > 0);
    if (len <= n) {
      return embed[--len];
    }
    else {
      return heap[--len - n];
    }
  }

  int length() const
    { return len; }
  bool isEmpty() const
    { return len==0; }
  bool isNotEmpty() const
    { return !isEmpty(); }

  // direct element access
  T const &getElt(int i) const
  {
    bc(i);
    if (i < n) {
      return embed[i];
    }
    else {
      return heap[i - n];
    }
  }

  T const& operator[] (int i) const
    { return getElt(i); }
  T & operator[] (int i)
    { return const_cast<T&>(getElt(i)); }

  T const &top() const
    { return getElt(len-1); }
  T &topNC()
    { return const_cast<T&>(getElt(len-1)); }

  void moveElement(int oldIndex, int newIndex);
};


template <class T, int n>
void ArrayStackEmbed<T,n>::moveElement(int oldIndex, int newIndex)
{
  this->bc(oldIndex);
  this->bc(newIndex);

  while (oldIndex < newIndex) {
    swap(this->operator[](oldIndex), this->operator[](oldIndex+1));
    oldIndex++;
  }

  while (oldIndex > newIndex) {
    swap(this->operator[](oldIndex), this->operator[](oldIndex-1));
    oldIndex--;
  }
}


// this code has not been tested, so it's #if 0'd out for now
#if 0
// STL-style iterator for ArrayStackEmbed
template <class T, int n>
class ArrayStackEmbedIter {
public:      // data
  // Array over which we're iterating.
  ArrayStackEmbed<T,n> *array;

  // Current index; changed as we increment.  'index' is
  // equal to array.length() for the "end" iterator.
  int index;

public:      // funcs
  ArrayStackEmbedIter(ArrayStackEmbed<T,n> &a, int i)
    : array(&a),
      index(i)
  {
    selfCheck();
  }

  ~ArrayStackEmbedIter()
  {}

  ArrayStackEmbedIter(ArrayStackEmbedIter const &obj)
    : DMEMB(array),
      DMEMB(index)
  {}

  ArrayStackEmbedIter& operator=(ArrayStackEmbedIter const &obj)
  {
    CMEMB(array);
    CMEMB(index);
    return *this;
  }

  // check internal invariants
  void selfCheck() const
  {
    xassert((unsigned)index <= array->length());
  }

  bool operator==(ArrayStackEmbedIter const &obj) const
  {
    return EMEMB(array) &&
           EMEMB(index);
  }

  bool operator<(ArrayStackEmbedIter const &obj) const
  {
    // Only legal to do inequality comparisons among iterators
    // pointing to the same container.
    xassert(array == obj.array);

    return index < obj.index;
  }

  RELATIONAL_OPERATORS(ArrayStackEmbedIter)

  ArrayStackEmbedIter const &operator++(/*preincrement*/)
  {
    index++;
    selfCheck();
  }

  ArrayStackEmbedIter operator++(int/*postincrement*/)
  {
    ArrayStackEmbedIter ret(*this);
    operator++();
    return ret;
  }
};
#endif // 0 (code not tested)

#endif // ARRAY_H
