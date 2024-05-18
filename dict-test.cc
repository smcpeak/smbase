// dict-test.cc
// Tests for some of the "dictionary" classes.

// At the moment these tests are not very exhaustive.  My immediate goal
// is simply to ensure all of the files generated from xstrobjdict.h
// compile.

#include "strintdict.h"                // StringIntDict
#include "strobjdict.h"                // StringObjDict
#include "strsobjdict.h"               // StringSObjDict

#include "sm-iostream.h"               // cout


static bool printIntKV(OldSmbaseString const &key, intptr_t value, void *)
{
  cout << "key=" << key << " value=" << value << endl;
  return false;
}


static void testStringIntDict()
{
  StringIntDict dict;
  xassert(dict.isEmpty());
  xassert(dict.size() == 0);

  xassert(!dict.isMapped("one"));
  dict.add("one", 1);
  xassert(!dict.isEmpty());
  xassert(dict.size() == 1);
  xassert(dict.isMapped("one"));

  dict.add("two", 2);
  xassert(!dict.isEmpty());
  xassert(dict.size() == 2);

  dict.add("three", 3);
  xassert(!dict.isEmpty());
  xassert(dict.size() == 3);

  dict.foreach(printIntKV);
}


static bool printObjKV(OldSmbaseString const &key, int *value, void *)
{
  cout << "key=" << key << " value=" << *value << endl;
  return false;
}


static void testStringObjDict()
{
  StringObjDict<int> dict;
  xassert(dict.isEmpty());
  xassert(dict.size() == 0);

  xassert(!dict.isMapped("one"));
  dict.add("one", new int(1));
  xassert(!dict.isEmpty());
  xassert(dict.size() == 1);
  xassert(dict.isMapped("one"));

  dict.add("two", new int(2));
  xassert(!dict.isEmpty());
  xassert(dict.size() == 2);

  dict.add("three", new int(3));
  xassert(!dict.isEmpty());
  xassert(dict.size() == 3);

  dict.foreach(printObjKV);
}


static void testStringSObjDict()
{
  StringSObjDict<int> dict;
  xassert(dict.isEmpty());
  xassert(dict.size() == 0);

  int one=1, two=2, three=3;

  xassert(!dict.isMapped("one"));
  dict.add("one", &one);
  xassert(!dict.isEmpty());
  xassert(dict.size() == 1);
  xassert(dict.isMapped("one"));

  dict.add("two", &two);
  xassert(!dict.isEmpty());
  xassert(dict.size() == 2);

  dict.add("three", &three);
  xassert(!dict.isEmpty());
  xassert(dict.size() == 3);

  dict.foreach(printObjKV);
}


void test_dict()
{
  testStringIntDict();
  testStringObjDict();
  testStringSObjDict();
}
