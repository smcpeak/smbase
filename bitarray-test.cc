// bitarray-test.cc
// Tests for bitarray.

#include "bitarray.h"                  // module under test

#include "exc.h"                       // xbase
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE


OPEN_ANONYMOUS_NAMESPACE


string toStringViaIter(BitArray const &b)
{
  stringBuilder sb;
  int index = 0;

  for (BitArray::Iter iter(b); !iter.isDone(); iter.adv()) {
    while (index < iter.data()) {
      sb << "0";
      index++;
    }
    sb << "1";
    index++;
  }

  while (index < b.length()) {
    sb << "0";
    index++;
  }

  return sb;
}


void testIter(char const *str)
{
  BitArray b = stringToBitArray(str);
  b.selfCheck();

  string s1 = toString(b);
  string s2 = toStringViaIter(b);
  if (s1 != s2 ||
      !stringEquals(s1, str)) {
    cout << "str: " << str << endl;
    cout << " s1: " << s1 << endl;
    cout << " s2: " << s2 << endl;
    xbase("testIter failed");
  }

  // also test the inverter
  BitArray c = ~b;
  c.selfCheck();

  stringBuilder inv;
  int len = strlen(str);
  for (int i=0; i<len; i++) {
    inv << (str[i]=='0'? '1' : '0');
  }

  string cStr = toString(c);
  if (!inv.equals(cStr)) {
    cout << " inv: " << inv << endl;
    cout << "cStr: " << cStr << endl;
    xbase("test inverter failed");
  }
}


void testUnionIntersection(char const *s1, char const *s2)
{
  int len = strlen(s1);
  xassert(len == (int)strlen(s2));

  BitArray b1 = stringToBitArray(s1);
  BitArray b2 = stringToBitArray(s2);

  stringBuilder expectUnion, expectIntersection;
  for (int i=0; i<len; i++) {
    expectUnion        << ((s1[i]=='1' || s2[i]=='1')? '1' : '0');
    expectIntersection << ((s1[i]=='1' && s2[i]=='1')? '1' : '0');
  }

  BitArray u = b1 | b2;
  BitArray i = b1 & b2;

  string uStr = toString(u);
  string iStr = toString(i);

  if (!stringEquals(uStr, expectUnion)) {
    cout << "         s1: " << s1 << endl;
    cout << "         s2: " << s2 << endl;
    cout << "       uStr: " << uStr << endl;
    cout << "expectUnion: " << expectUnion << endl;
    xbase("test union failed");
  }

  if (!stringEquals(iStr, expectIntersection)) {
    cout << "                s1: " << s1 << endl;
    cout << "                s2: " << s2 << endl;
    cout << "              iStr: " << iStr << endl;
    cout << "expectIntersection: " << expectIntersection << endl;
    xbase("test intersection failed");
  }
}


void testAnyEvenOddBitPair(char const *s, bool expect)
{
  BitArray b = stringToBitArray(s);
  bool answer = b.anyEvenOddBitPair();
  if (answer != expect) {
    static char const *boolName[] = { "false", "true" };
    cout << "     s: " << s << endl;
    cout << "answer: " << boolName[answer] << endl;
    cout << "expect: " << boolName[expect] << endl;
    xbase("test anyEvenOddBitPair failed");
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_bitarray()
{
        //            1111111111222222222233333333334444444444555555555566
        //  01234567890123456789012345678901234567890123456789012345678901
  testIter("00000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000111111111111000000000000");
  testIter("0000000000000000000000000000000000000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000000111111111111000000000000");
  testIter("0000000000000000000000000000000000000000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000000000111111111111000000000000");
  testIter("0000000000000000000000000000000000000000000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000000000000111111111111000000000000");

  testIter("0101");
  testIter("1");
  testIter("0");
  testIter("");
  testIter("1111");
  testIter("0000");
  testIter("000000000000111111111111000000000000");
  testIter("111111111111111000000000000011111111");
  testIter("10010110010101010100101010101010100110001000100001010101111");

  testUnionIntersection("",
                        "");

  testUnionIntersection("1",
                        "0");

  testUnionIntersection("10",
                        "00");

  testUnionIntersection("1001000100111110101001001001011111",
                        "0001100101011101011010000111010110");

  testUnionIntersection("1111111111111111111111111111111111",
                        "0000000000000000000000000000000000");

  testUnionIntersection("0000111111000001111110000011110000",
                        "1111000000111110000001111100001111");

  testAnyEvenOddBitPair("0000", false);
  testAnyEvenOddBitPair("0001", false);
  testAnyEvenOddBitPair("0010", false);
  testAnyEvenOddBitPair("0100", false);
  testAnyEvenOddBitPair("1000", false);
  testAnyEvenOddBitPair("0110", false);
  testAnyEvenOddBitPair("1110", true);
  testAnyEvenOddBitPair("0111", true);
  testAnyEvenOddBitPair("1111", true);
  testAnyEvenOddBitPair("11110", true);
  testAnyEvenOddBitPair("01100", false);
}


// EOF
