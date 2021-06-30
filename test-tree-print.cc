// test-tree-print.cc
// Test code for tree-print module.

#include "tree-print.h"                // module under test

#include "sm-test.h"                   // USUAL_MAIN


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

  tp << tp.br;

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
     << "end;" << tp.br;
  tp.end();

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
     << "d;" << tp.br;
  tp.end();

  printWithRuler(tp, 20);
}


static void unindentLabel()
{
  TreePrint tp;

  tp.begin(0);
  tp << "int f()" << tp.br;
  tp.begin(2);
  tp << "{" << tp.br;
  tp <<   "int x;" << tp.br
     <<   "x = 8;" << tp.br;
  tp << tp.und << "label:" << tp.br;
  tp <<   "x++;" << tp.br
     <<   "goto label;" << tp.br;
  tp.end();
  tp << "}" << tp.br;
  tp.end();

  printWithRuler(tp, 20);
}


static void simpleCFunction()
{
  TreePrint tp;

  tp.begin(0);
  tp << "int f()" << tp.br;
  tp.begin(2);
  tp << "{" << tp.br;
  tp <<   "return 0;" << tp.br;
  tp.end();
  tp << "}" << tp.br;
  tp.end();

  //debugPrint(tp);
  printWithRuler(tp, 20);
}


// Based on elsa/test/pprint/longlines2.c.
static void complexPrintfCall()
{
  TreePrint tp;

  tp << "void f()" << tp.br;
  tp.begin(2);
    tp << "{" << tp.br;
    tp <<   "av_oo_pointer_t __ptr_to_p;" << tp.br;

    tp.begin();
      tp << "printf(" << tp.optbr
         <<   "\"**pp=%d pp=%s\\n\"," << tp.sp
         <<   "*((int *)";
      tp.begin();
        tp <<          "av_oo_ptr_check(" << tp.optbr
           <<            "*((av_oo_pointer_t *)";
        tp.begin();
          tp <<                               "av_oo_ptr_check("
             <<                                 "pp," << tp.sp
             <<                                 "16)";
        tp.end();
        tp <<             ")," << tp.sp
           <<            "4)";
      tp.end();
      tp <<    ")," << tp.sp;
      tp.beginConsistent();    // for the ?:
        tp.begin();
          tp << "av_oo_ptr_cmp_eq(" << tp.optbr
             <<   "pp," << tp.sp
             <<   "__ptr_to_p)";
        tp.end();
        tp << "?" << tp.sp
           << "\"&p\" :" << tp.sp
           << "(";
        tp.beginConsistent();    // for the ?:
          tp.begin();
            tp << "av_oo_ptr_cmp_eq(" << tp.optbr
               <<   "pp," << tp.sp
               <<   "__ptr_to_q)";
          tp.end();
          tp << "?" << tp.sp
             << "\"&q\" :" << tp.sp
             << "\"?\"";
        tp.end();
        tp << "))";
      tp.end();
    tp.end();
    tp << ";" << tp.br;

  tp.end();
  tp << "}" << tp.br;

  printWithRuler(tp, 72);
  printWithRuler(tp, 40);
}


static void arrayInit1(bool consistent)
{
  TreePrint tp;

  if (consistent) {
    tp.beginConsistent();
  }
  else {
    tp.begin();
  }
  tp << "int arr[] = {" << tp.br
     <<   "1," << tp.br
     <<   "2," << tp.br
     <<   "3," << tp.br
     <<   "4"
     << tp.br << tp.und << "};" << tp.br;
  tp.end();

  printWithRuler(tp, 20);
}


static void arrayInit2(bool consistent)
{
  TreePrint tp;

  if (consistent) {
    tp.beginConsistent();
  }
  else {
    tp.begin();
  }
  tp << "int arr[] = {" << tp.sp
     <<   "1," << tp.sp
     <<   "2," << tp.sp
     <<   "3," << tp.sp
     <<   "4"
     << tp.sp << tp.und << "};" << tp.br;
  tp.end();

  printWithRuler(tp, 20);
}


static void arrayInit3()
{
  TreePrint tp;

  tp.beginConsistent();
  tp << "int arr[] = {" << tp.sp;

  tp.begin(0);
  tp << "1," << tp.sp;
  tp << "1," << tp.sp;
  tp << "1," << tp.sp;

  tp.begin();
  tp << '{' << tp.sp;
  tp << "1," << tp.sp;
  tp << "1," << tp.sp;
  tp << "1" << tp.sp;
  tp << "},";
  tp.end();
  tp << tp.sp;

  tp << "1," << tp.sp;
  tp << "1," << tp.sp;
  tp << "1," << tp.sp;
  tp << "1";
  tp.end();

  tp << tp.sp << tp.und << "};" << tp.br;
  tp.end();

  printWithRuler(tp, 50);
  printWithRuler(tp, 40);
  printWithRuler(tp, 30);
  printWithRuler(tp, 20);
  printWithRuler(tp, 10);
}


void entry()
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
  unindentLabel();
  simpleCFunction();
  complexPrintfCall();
  arrayInit1(false);
  arrayInit1(true);
  arrayInit2(false);
  arrayInit2(true);
  arrayInit3();
}


USUAL_MAIN


// EOF
