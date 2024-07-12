// array-test.cc
// Test for for 'array' module.

#include "array.h"                     // module under test

// this dir
#include "compare-util.h"              // compare
#include "exc.h"                       // XBase
#include "objlist.h"                   // ObjList
#include "sm-iostream.h"               // ostream
#include "sm-macros.h"                 // TABLESIZE
#include "sm-test.h"                   // PVAL, DIAG, verbose
#include "str.h"                       // string
#include "stringb.h"                   // stringb
#include "xassert.h"                   // xassert

// libc++
#include <algorithm>                   // std::sort
#include <cstdlib>                     // std::rand
#include <vector>                      // std::vector

// libc
#include <assert.h>                    // assert

using namespace smbase;


// Assert that 'ase' and 'vec' have the same sequence of elements.
template <class T, int n>
static void checkEqual(ArrayStackEmbed<T,n> const &ase,
                       std::vector<T> const &vec)
{
  xassert(ase.size() == vec.size());
  xassert(ase.empty() == vec.empty());

  for (size_t i=0; i < ase.size(); i++) {
    xassert(ase[i] == vec[i]);
  }
}


// Test that std::sort works on ArrayStackEmbed.
static void testEmbedArraySort()
{
  ArrayStackEmbed<int,2> ase;
  std::vector<int> vec;

  // One specific input.
  for (int i : (int[]){3,5,4}) {
    ase.push_back(i);
    vec.push_back(i);
  }
  checkEqual(ase, vec);

  // Sort both.
  std::sort(ase.begin(), ase.end());
  std::sort(vec.begin(), vec.end());
  checkEqual(ase, vec);

  // Clear both.
  ase.clear();
  vec.clear();
  checkEqual(ase, vec);

  // A bunch of random inputs.
  for (int len=0; len < 10; len++) {
    int iterCount = len*5 + 1;
    for (int iter=0; iter < iterCount; iter++) {
      // Also test with a fresh array.
      ArrayStackEmbed<int,2> ase2;

      // And another, with a different size.
      ArrayStackEmbed<int,5> ase3;

      // Populate the arrays.
      for (int i=0; i < len; i++) {
        int num = std::rand() % 20;
        ase.push_back(num);
        ase2.push_back(num);
        ase3.push_back(num);
        vec.push_back(num);
      }
      checkEqual(ase, vec);
      checkEqual(ase2, vec);
      checkEqual(ase3, vec);

      // Sort.
      std::sort(ase.begin(), ase.end());
      std::sort(ase2.begin(), ase2.end());
      std::sort(ase3.begin(), ase3.end());
      std::sort(vec.begin(), vec.end());
      checkEqual(ase, vec);
      checkEqual(ase2, vec);
      checkEqual(ase3, vec);

      // Clear.
      ase.clear();
      ase2.clear();
      ase3.clear();
      vec.clear();
      checkEqual(ase, vec);
      checkEqual(ase2, vec);
      checkEqual(ase3, vec);
    }
  }
}


static void testAsVector()
{
  ArrayStack<int> stk;
  xassert(stk.asVector() == std::vector<int>{});

  stk.push(1);
  xassert(stk.asVector() == std::vector<int>{1});

  stk.push(2);
  xassert((stk.asVector() == std::vector<int>{1, 2}));
}


// 2024-05-19: The test code above this point was in this file,
// 'array-test.cc'.  The test code below this point was in another file,
// 'testarray.cc'.  Both files were intended to test the 'array' module,
// but evidently when I created the second one I did not realize the
// first existed.  So, now they are combined.  I'm leaving this note in
// part to explain the stylistic shift from above to below.


static int maxLength = 0;

// Return index of first element in 'list' that is equal, after
// dereferencing, to 't', or -1 if none.
template <class T>
static int indexOfFirstDeref(ObjList<T> const &list, T const &t)
{
  int index = 0;
  FOREACH_OBJLIST(T, list, iter) {
    if (*(iter.data()) == t) {
      return index;
    }
    index++;
  }
  return -1;
}

template <class T>
static int reversedIndexDerefHelper(ObjListIter<T> const &iter, int index, T const &t)
{
  if (iter.isDone()) {
    xassert(index == -1);
    return -1;
  }
  else {
    xassert(index >= 0);
    ObjListIter<T> next(iter);
    next.adv();
    int tailIndex = reversedIndexDerefHelper(next, index-1, t);
    if (tailIndex >= 0) {
      return tailIndex;
    }
    else if (*(iter.data()) == t) {
      return index;
    }
    else {
      return -1;
    }
  }
}

// Return 'indexOfFirstDeref(list.reverse())', except without
// actually reversing the list.
template <class T>
static int reversedIndexOfFirstDeref(ObjList<T> const &list, T const &t)
{
  ObjListIter<T> iter(list);
  return reversedIndexDerefHelper(iter, list.count()-1, t);
}

// This is a candidate for moving into xobjlist.h or perhaps another
// file for general-purpose use.  The issue with the former is the
// dependence on ostream.
template <class T>
static ostream& printList(ostream &os, ObjList<T> const &list)
{
  os << '[';
  FOREACH_OBJLIST(T, list, iter) {
    os << ' ' << *(iter.data());
  }
  if (list.isNotEmpty()) {
    os << ' ';
  }
  os << ']';
  return os;
}

template <class T>
static inline ostream& operator<< (ostream &os, ObjList<T> const &list)
{
  return printList(os, list);
}

template <class T>
static void moveListElement(ObjList<T> &list, int oldIndex, int newIndex)
{
  T *t = list.removeAt(oldIndex);
  list.insertAt(t, newIndex);
}

// This is a candidate for moving into array.h or perhaps another file
// for general-purpose use.  The issue with the former is the
// dependence on ostream.
template <class T>
static ostream& printArray(ostream &os, ArrayStack<T> const &array)
{
  os << '[';
  for (int i=0; i < array.length(); i++) {
    os << ' ' << array[i];
  }
  if (array.isNotEmpty()) {
    os << ' ';
  }
  os << ']';
  return os;
}

template <class T>
static inline ostream& operator<< (ostream &os, ArrayStack<T> const &array)
{
  return printArray(os, array);
}


// one round of testing
static void round(int ops)
{
  // implementations to test
  ArrayStack<int> arrayStack;
  ArrayStackEmbed<int, 10> arrayStackEmbed;

  // "trusted" implementation to compare with
  ObjList<int> listStack;

  while (ops--) {
    // check that the arrays and list agree
    {
      int length = listStack.count();
      if (length > 0) {
        xassert(listStack.first()[0] == arrayStack.top());
        xassert(listStack.first()[0] == arrayStackEmbed.top());
      }

      int index = length-1;
      FOREACH_OBJLIST(int, listStack, iter) {
        int item = *(iter.data());
        xassert(item == arrayStack[index]);
        xassert(item == arrayStackEmbed[index]);
        if (reversedIndexOfFirstDeref(listStack, item) == arrayStack.indexOf(item)) {
          // fine
        }
        else {
          PVAL(listStack);
          PVAL(arrayStack);
          PVAL(index);
          PVAL(item);
        }
        xassert(reversedIndexOfFirstDeref(listStack, item) == arrayStack.indexOf(item));
        index--;
      }
      xassert(index == -1);
      xassert(length == arrayStack.length());
      xassert(length == arrayStackEmbed.length());
      xassert(arrayStack.isEmpty() == listStack.isEmpty());
      xassert(arrayStackEmbed.isEmpty() == listStack.isEmpty());
      xassert(arrayStack.isNotEmpty() == listStack.isNotEmpty());
      xassert(arrayStackEmbed.isNotEmpty() == listStack.isNotEmpty());

      if (length > maxLength) {
        maxLength = length;
      }
    }

    // do a random operation
    int op = rand() % 120;
    if (op < 40 && arrayStack.isNotEmpty()) {
      // pop
      int i = arrayStack.pop();
      int j = arrayStackEmbed.pop();
      int *k = listStack.removeFirst();
      xassert(i == *k);
      xassert(j == *k);
      delete k;
    }
    else if (op < 60 && arrayStack.isNotEmpty()) {
      // moveElement
      int oldIndex = rand() % (arrayStack.length());
      int newIndex = rand() % (arrayStack.length());
      arrayStack.moveElement(oldIndex, newIndex);
      arrayStackEmbed.moveElement(oldIndex, newIndex);
      oldIndex = arrayStack.length()-1 - oldIndex;
      newIndex = arrayStack.length()-1 - newIndex;
      moveListElement(listStack, oldIndex, newIndex);
    }
    else {
      // push
      int elt = rand() % 100;
      arrayStack.push(elt);
      arrayStackEmbed.push(elt);
      listStack.prepend(new int(elt));
    }
  }
}


static void testArrayNegativeLength()
{
  // This should be allowed.
  Array<char> arrZeroLength(0);

  try {
    DIAG("This should throw:");
    Array<char> arr(-700);
    assert(!"should have failed");
  }
  catch (XBase &x) {
    DIAG("as expected: " << x.why());
  }
}


static bool isOdd(int i)
{
  return (i % 2) == 1;
}

static bool isEven(int i)
{
  return (i % 2) == 0;
}

static bool isDivis3(int i)
{
  return (i % 3) == 0;
}

static bool isNotDivis3(int i)
{
  return !isDivis3(i);
}

static void checkEqual(ArrayStack<int> const &arr, int *expect, int expectLen)
{
  xassert(arr.length() == expectLen);
  for (int i=0; i < expectLen; i++) {
    xassert(arr[i] == expect[i]);
  }
}

static void testOneApplyFilter(bool (*condition)(int), int *expect, int expectLen)
{
  ArrayStack<int> arr;
  for (int i=0; i < 10; i++) {
    arr.push(i);
  }

  applyFilter(arr, condition);

  checkEqual(arr, expect, expectLen);
}

static void testApplyFilter()
{
  {
    int expect[] = { 0, 2, 4, 6, 8 };
    testOneApplyFilter(isEven, expect, TABLESIZE(expect));
  }

  {
    int expect[] = { 1, 3, 5, 7, 9 };
    testOneApplyFilter(isOdd, expect, TABLESIZE(expect));
  }

  {
    int expect[] = { 0, 3, 6, 9 };
    testOneApplyFilter(isDivis3, expect, TABLESIZE(expect));
  }

  {
    int expect[] = { 1, 2, 4, 5, 7, 8 };
    testOneApplyFilter(isNotDivis3, expect, TABLESIZE(expect));
  }
}


static void testSort()
{
  for (int j=0; j < 10; ++j) {
    ArrayStack<string> names;

    // These are added in numeric order but not string order, so the
    // 'sort' call has something non-trivial to do.
    for (int i=0; i < 1000; ++i) {
      names.push(stringb(i));
    }

    // Partly this tests whether 'sort' puts the objects into the right
    // order.  But it also tests that it does not do anything it
    // shouldn't in terms of how objects get copied.  For example, the
    // old qsort-based implementation breaks with std::string, and this
    // test can detect that.
    names.sort([](string const *a, string const *b) {
                 return compare(*a, *b);
               });

    for (int i=1; i < 1000; ++i) {
      xassert(names[i-1] < names[i]);
    }
  }
}


// Called by unit-tests.cc.
void test_array()
{
  // With the optimizer disabled, the test takes about 1s to run
  // two iterations, so at 5, this takes about 2.5s.
  //
  // 2024-05-19: I'm reducing the inner count from 1000 to 100 to speed
  // up the tests.  That makes the above time estimates incorrect.
  // There is no reason to suspect issues in this code, and new code
  // should be using std::vector instead anyway.
  //
  for (int i=0; i<5; i++) {
    round(100);
  }

  testEmbedArraySort();
  testAsVector();
  testArrayNegativeLength();
  testApplyFilter();
  testSort();
}


// EOF
