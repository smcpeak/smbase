// growbuf-test.cc
// Tests for growbuf.

#include "growbuf.h"                   // module under test

#include "xassert.h"                   // xfailure

#include <string.h>                    // memcmp


// Called from unit-tests.cc.
void test_growbuf()
{
  unsigned char const str[] = "crazy like a mad cow!";
  size_t len = sizeof(str);

  GrowBuffer buf;
  smbase_loopi(10) {
    buf.append(str, len);
  }
  smbase_loopi(10) {
    if (0!=memcmp(str, buf.getData()+len*i, len)) {
      xfailure("buffer contents are wrong");
    }
  }
}


// EOF
