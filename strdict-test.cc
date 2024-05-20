// strdict-test.cc
// Tests for strdict.

#include "strdict.h"                   // module under test

#include <stdlib.h>                    // rand

#define myrandom(n) (rand()%(n))


namespace {

char randChar()
{
  return (char)(myrandom(127-32+1)+32);
}

string randString(int len)
{
  stringBuilder str;
  loopj(len) {
    str << randChar();
  }
  return str.str();
}

string randStringRandLen(int maxlen)
{
  return randString(myrandom(maxlen)+1);
}

string randKey(StringDict const &dict)
{
  int size = dict.size();
  xassert(size > 0);

  int nth = myrandom(size);
  StringDict::IterC entry(dict);
  for (; nth > 0; entry.next(), nth--)
    {}

  return entry.key();
}

} // anonymous namespace


// Called from unit-tests.cc.
void test_strdict()
{
  StringDict dict;
  int size=0, collisions=0;

  int iters = 1000;
  loopi(iters) {
    switch (myrandom(6)) {
      case 0: {
        // insert a random element
        string key = randStringRandLen(10);
        string value = randStringRandLen(30);

        if (!dict.isMapped(key.c_str())) {
          dict.add(key.c_str(), value.c_str());
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
        StringDict dict2(dict);
        xassert(dict2 == dict);
        xassert(dict2.size() == dict.size());

        // modify it, then verify inequality
        if (!dict2.isEmpty()) {
          string key = randKey(dict2);
          string value = dict2.queryf(key.c_str());

          if (myrandom(2) == 0) {
            dict2.remove(key.c_str());
          }
          else {
            dict2.modify(key.c_str(), stringbc(value << "x"));
          }
          xassert(dict2 != dict);
        }

        break;
      }

      case 5: {
        // random modification
        if (!dict.isEmpty()) {
          string key = randKey(dict);
          dict.modify(key.c_str(), randStringRandLen(30).c_str());
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
