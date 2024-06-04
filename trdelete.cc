// trdelete.cc            see license.txt for copyright and terms of use
// code for trdelete.h

// When compiling with Clang-16 against the GCC-9 libc++, I need to set
// this to get the `operator delete` functions that take a `size_t`
// argument.
#define __cpp_sized_deallocation 1

#include "trdelete.h"     // this module

#include "breaker.h"      // breaker

#include <new>            // operator delete

#include <stdlib.h>       // abs
#include <string.h>       // memset

// There is a surprising twist to our story.  When an object is created with
// 'new' on the dynamic heap, and it throws an exception, its destructor is not
// called, because the object is not fully constructed yet.  However, the
// storage allocated for that object should be freed; the exception runtimes
// do this.

// But, there is a bug (under Borland C++ 4.5) when operator delete is called
// under these circumstances, the size argument is wrong.  I've been getting
// values like 0x12fc14, which are pointers to objects on the stack.  After
// quite a bit of dump tracing, I can't find an easy way to turn this pointer
// into the desired value.

// However, the dynamic heap node format for the Borland runtimes is pretty
// simple, and an estimate of the heap size can readily be determined; the
// dword just before the passed block pointer has such an estimate.  Therefore,
// my strategy is to compare the given size with the dword at offset -4, and if
// they are within 64 of each other (I've never seen differences greater than
// 16), the size argument is considered valid.  If they aren't there isn't a
// reliable way to convert the estimate into a precise size, so I just skip
// trashing the memory altogether.


static void trash(void *blk, size_t size)
{
  #ifdef __BORLANDC__
  long guess = ((long*)blk)[-1];
  if (abs(guess - (long)size) > 64) {
    // assume the size is bogus
    breaker();     // I want to know about it, for now
    return;
  }
  #endif

  // assume the size is ok
  memset(blk, 0xAA, size);
    // the choice of AA is made as it is easily recognizable,
    // and 0xAAAAAAAA is pretty much always a bad pointer
}


void trashingDelete(void *blk, size_t size)
{
  trash(blk, size);

  // use the global delete operator to free the memory;
  // gratuitous cast to char* to silence gcc warning
  // "not a pointer-to-object type"
  //
  // Passing `size` is needed to pacify Address Sanitizer.
  ::operator delete((char*)blk, size);
}


void trashingDeleteArr(void *blk, size_t size)
{
  trash(blk, size);

  // use the global delete operator to free the memory;
  // (see comment about gratuitious cast, above)
  ::operator delete[]((char*)blk, size);
}


// EOF
