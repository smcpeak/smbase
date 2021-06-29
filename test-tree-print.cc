// test-tree-print.cc
// Test code for tree-print module.

#include "tree-print.h"                // module under test


static void debugPrint(TreePrint &tp)
{
  // So the debug print will include lengths.
  tp.m_root.scan();
  tp.debugPrintCout();
}


static void printWithRuler(TreePrint &tp, int margin)
{
  xassert(tp.allSequencesClosed());

  cout << '|';
  for (int i=0; i < margin-2; i++) {
    cout << '-';
  }
  cout << "| margin=" << margin << '\n';

  tp.print(cout, margin);
}


// Very simple starting test.
static void test1()
{
  TreePrint tp;
  tp << tp.seq << "hello," << tp.sp << "world!" << tp.end << tp.br;
  debugPrint(tp);
  printWithRuler(tp, 20);
  printWithRuler(tp, 10);
}


// Example from Oppen paper.
static void test2()
{
  TreePrint tp;

  tp << tp.seq
     << "var" << tp.sp << "x: integer," << tp.sp
                       << "y: char;"
     << tp.end
     << tp.sp;

  tp << tp.seq
     << "begin" << tp.sp
     << "x := 1;" << tp.sp
     << "y := 'a';"
     << tp.end
     << tp.sp << "end"
     << tp.br;

  //debugPrint(tp);
  printWithRuler(tp, 30);
  printWithRuler(tp, 20);
  printWithRuler(tp, 10);
}


// Example from Oppen paper.
static void test3()
{
  TreePrint tp;

  tp << tp.seq;

  tp << tp.seq
     << "f(a," << tp.sp << "b," << tp.sp << "c," << tp.sp << "d)"
     << tp.end;

  tp << tp.sp << "+" << tp.sp;

  tp << tp.seq
     << "g(a," << tp.sp << "b," << tp.sp << "c," << tp.sp << "d)"
     << tp.end;

  tp << tp.end << tp.br;

  //debugPrint(tp);
  printWithRuler(tp, 30);
  printWithRuler(tp, 25);
  printWithRuler(tp, 20);
  printWithRuler(tp, 10);
  printWithRuler(tp, 8);
}


// Exercise operator<< accepting 'int' and 'char'.
static void test4()
{
  TreePrint tp;

  tp << "x=" << 3 << "," << tp.sp << "c='" << 'x' << "'" << tp.br;

  printWithRuler(tp, 10);
  printWithRuler(tp, 5);
}


// Test that we can handle unclosed sequences.
static void testUnclosedSeq()
{
  TreePrint tp;

  tp << tp.seq_noind << "hi" << tp.br;
  xassert(!tp.allSequencesClosed());
  tp.print(cout);

  tp.clear();
  tp << tp.seq_noind << "hi" << tp.br;
  xassert(!tp.allSequencesClosed());
  tp.print(cout);
}


int main()
{
  test1();
  test2();
  test3();
  test4();
  testUnclosedSeq();
  return 0;
}


// EOF
