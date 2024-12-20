// trdelete.h            see license.txt for copyright and terms of use
// An 'operator delete' which overwrites the deallocated memory with
// 0xAA before deallocating it.

// This module doesn't work under msvc, I don't care to figure out why.
//
// It also seems to not work under Clang 16 now.  I thought it did?  I'm
// close to removing the module entirely but let's try the minimal
// fix...
//
#if defined(_MSC_VER) || defined(__clang__)
  #define TRDELETE_H                 // Make it omit this file.
  #define TRASHINGDELETE             // Uses are a no-op.
  #define TRASHINGDELETE_DISABLED    // Signal to not run the test.
#endif

#ifndef TRDELETE_H
#define TRDELETE_H

#include <stddef.h>      // size_t

void trashingDelete(void *blk, size_t size);
void trashingDeleteArr(void *blk, size_t size);

// to use, include the TRASHINGDELETE macro in the public section of a class

#define TRASHINGDELETE                                                              \
  void operator delete(void *blk, size_t size) { trashingDelete(blk, size); }       \
  void operator delete[](void *blk, size_t size) { trashingDeleteArr(blk, size); }

#endif // TRDELETE_H
