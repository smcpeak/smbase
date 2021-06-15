// testarray.cc
// test the array.h module
// based on tarrayqueue.cc

#include "array.h"                     // module to test
#include "ckheap.h"                    // malloc_stats
#include "objlist.h"                   // ObjList
#include "sm-macros.h"                 // TABLESIZE
#include "test.h"                      // PVAL, USUAL_MAIN

#include <assert.h>                    // assert
#include <stdio.h>                     // printf
#include <stdlib.h>                    // exit


int maxLength = 0;

// Return index of first element in 'list' that is equal, after
// dereferencing, to 't', or -1 if none.
template <class T>
int indexOfFirstDeref(ObjList<T> const &list, T const &t)
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
int reversedIndexDerefHelper(ObjListIter<T> const &iter, int index, T const &t)
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
int reversedIndexOfFirstDeref(ObjList<T> const &list, T const &t)
{
  ObjListIter<T> iter(list);
  return reversedIndexDerefHelper(iter, list.count()-1, t);
}

// This is a candidate for moving into xobjlist.h or perhaps another
// file for general-purpose use.  The issue with the former is the
// dependence on ostream.
template <class T>
ostream& printList(ostream &os, ObjList<T> const &list)
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
inline ostream& operator<< (ostream &os, ObjList<T> const &list)
{
  return printList(os, list);
}

template <class T>
void moveListElement(ObjList<T> &list, int oldIndex, int newIndex)
{
  T *t = list.removeAt(oldIndex);
  list.insertAt(t, newIndex);
}

// This is a candidate for moving into array.h or perhaps another file
// for general-purpose use.  The issue with the former is the
// dependence on ostream.
template <class T>
ostream& printArray(ostream &os, ArrayStack<T> const &array)
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
inline ostream& operator<< (ostream &os, ArrayStack<T> const &array)
{
  return printArray(os, array);
}


// one round of testing
void round(int ops)
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
    cout << "This should throw:" << endl;
    Array<char> arr(-700);
    assert(!"should have failed");
  }
  catch (xBase &x) {
    cout << "as expected: " << x.why() << endl;
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


void entry()
{
  // With the optimizer disabled, the test takes about 1s to run
  // two iterations, so at 5, this takes about 2.5s.
  for (int i=0; i<5; i++) {
    round(1000);
  }

  testArrayNegativeLength();
  testApplyFilter();

  malloc_stats();
  printf("arrayStack appears to work; maxLength=%d\n", maxLength);
}

USUAL_MAIN


// EOF
