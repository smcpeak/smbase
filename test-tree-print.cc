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
  tp.begin();
  tp << "hello," << tp.sp << "world!";
  tp.end();
  tp << tp.br;
  debugPrint(tp);
  printWithRuler(tp, 20);
  printWithRuler(tp, 10);
}


// Example from Oppen paper.
static void test2()
{
  TreePrint tp;

  tp.begin();
  tp << "var" << tp.sp << "x: integer," << tp.sp << "y: char;";
  tp.end();

  tp << tp.sp;

  tp.begin();
  tp << "begin" << tp.sp
     << "x := 1;" << tp.sp
     << "y := 'a';";
  tp.end();
  tp << tp.sp << "end"
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

  tp.begin();

  tp.begin();
  tp << "f(a," << tp.sp << "b," << tp.sp << "c," << tp.sp << "d)";
  tp.end();

  tp << tp.sp << "+" << tp.sp;

  tp.begin();
  tp << "g(a," << tp.sp << "b," << tp.sp << "c," << tp.sp << "d)";
  tp.end();

  tp.end();
  tp << tp.br;

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

  tp.begin(0);
  tp << "hi" << tp.br;
  xassert(!tp.allSequencesClosed());
  tp.print(cout);

  tp.clear();
  tp.begin(0);
  tp << "hi" << tp.br;
  xassert(!tp.allSequencesClosed());
  tp.print(cout);
}


// Sequence with indentation other than 2.
static void testVariableIndent()
{
  TreePrint tp;

  tp.begin(6);
  tp << "cases 1:..." << tp.br
     << "2:..." << tp.br
     << "3:...";
  tp.end();
  tp << tp.br;

  printWithRuler(tp, 12);
}


static void consistentBreaks1(bool consistentBreaks)
{
  TreePrint tp;

  tp.begin(2 /*ind*/, consistentBreaks);
  tp << "begin" << tp.sp
     << "x := f(x);" << tp.sp
     << "y := f(y);" << tp.sp
     << "z := f(z);" << tp.sp
     << "w := f(w);" << tp.sp
     << "end;";
  tp.end();
  tp << tp.br;

  printWithRuler(tp, 30);
}

static void consistentBreaks2(bool consistentBreaks)
{
  TreePrint tp;

  tp.begin(7 /*ind*/, consistentBreaks);
  tp << "locals x," << tp.sp
     << "y," << tp.sp
     << "z," << tp.sp
     << "w," << tp.sp
     << "a," << tp.sp
     << "b," << tp.sp
     << "c," << tp.sp
     << "d;";
  tp.end();
  tp << tp.br;

  printWithRuler(tp, 20);
}


int main()
{
  test1();
  test2();
  test3();
  test4();
  testUnclosedSeq();
  testVariableIndent();
  consistentBreaks1(true);
  consistentBreaks1(false);
  consistentBreaks2(true);
  consistentBreaks2(false);
  return 0;
}


// EOF
