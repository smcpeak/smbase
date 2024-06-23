// call-abort.cc
// Program that just calls 'abort()'.

// This is used by run-process-test.cc.

#include <stdlib.h>                    // abort


int main()
{
  abort();
  return 4;        // not reached
}


// EOF
