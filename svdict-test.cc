// svdict-test.cc
// Tests for svdict.

#include "svdict.h"                    // module under test

#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-stdint.h"                 // intptr_t

#include <stdlib.h>                    // rand

#define myrandom(n) (rand()%(n))


OPEN_ANONYMOUS_NAMESPACE

char randChar()
{
  return (char)(myrandom(127-32+1)+32);
}

string randString(int len)
{
  stringBuilder str;
  smbase_loopj(len) {
    str << randChar();
  }
  return str.str();
}

string randStringRandLen(int maxlen)
{
  return randString(myrandom(maxlen)+1);
}

string randKey(StringVoidDict const &dict)
{
  int size = dict.size();
  xassert(size > 0);

  int nth = myrandom(size);
  StringVoidDict::IterC entry(dict);
  for (; nth > 0; entry.next(), nth--)
    {}

  return entry.key();
}

void *randVoidPtr()
{
  return (void*)(intptr_t)(myrandom(100) * 8);
}

CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_svdict()
{
  StringVoidDict dict;
  int size=0, collisions=0;

  int iters = 1000;
  smbase_loopi(iters) {
    switch (myrandom(6)) {
      case 0: {
        // insert a random element
        string key = randStringRandLen(10);
        void *value = randVoidPtr();

        if (!dict.isMapped(key.c_str())) {
          dict.add(key.c_str(), value);
          size++;
        }
        else {
          collisions++;
        }
        break;
      }

      case 1: {
        // remove a random element
        if (dict.isEmpty()) {
          break;
        }

        string key = randKey(dict);
        dict.remove(key.c_str());
        size--;
        break;
      }

      case 2: {
        // check a random element that should not be there
        string key = randStringRandLen(10);
        if (dict.isMapped(key.c_str())) {
          collisions++;
        }
        break;
      }

      case 3: {
        // verify that computed length is right
        xassert(size == dict.size());
        break;
      }

      case 4: {
        // test == and =
        StringVoidDict dict2(dict);
        xassert(dict2 == dict);
        xassert(dict2.size() == dict.size());

        // modify it, then verify inequality
        if (!dict2.isEmpty()) {
          string key = randKey(dict2);
          void *value = dict2.queryf(key.c_str());

          if (myrandom(2) == 0) {
            dict2.remove(key.c_str());
          }
          else {
            dict2.modify(key.c_str(), (void*)((char const *)value + 24));
          }
          xassert(dict2 != dict);
        }

        break;
      }

      case 5: {
        // random modification
        if (!dict.isEmpty()) {
          string key = randKey(dict);
          dict.modify(key.c_str(), randVoidPtr());
        }
        break;
      }

      default:
        xfailure("huh?");
        break;
    }
  }

  cout << "final size: " << size
       << "\ncollisions: " << collisions
       << "\n";

  cout << "all tests passed\n";
}


// EOF
