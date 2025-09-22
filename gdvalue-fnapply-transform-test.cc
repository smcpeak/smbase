// gdvalue-fnapply-transform-test.cc
// Tests for `gdvalue-fnapply-transform` module.

#include "smbase/gdvalue-fnapply-transform.h"    // module under test

#include "smbase/gdvalue-set.h"                  // gdv::toGDValue(std::set)
#include "smbase/gdvalue.h"                      // gdv::{GDValue, fromGDVN}
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ[_GDV]

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


void test_basics()
{
  GDValue commands = fromGDVN(R"(
    [
      123
      Define(x 456)
      x
      Define(f(n) (n n))
      f(2)
      f(x)
    ]
  )");

  GDValueFnApplyTransform transform;
  transform.selfCheck();

  GDValue actual = transform.transform(commands);
  transform.selfCheck();

  EXPECT_EQ_GDV(actual, fromGDVN(R"(
    [
      123         // Not a defined var/func, so self-evaluating.
      null        // Result of `Define`.
      456         // Transformation of defined variable `x`.
      null        // Result of `Define`.
      (2 2)       // Transformation of call to defined function `f`.
      (456 456)   // Same, but with a transformed argument.
    ]
  )"));

  // The substitution process preserves argument locations at the
  // expense of parameter locations.
  EXPECT_EQ(actual.dumpToString(), R"(/*2:5*/[
  /*3:7*/123
  null
  /*4:16*/456
  null
  /*6:19*/(/*7:9*/2 /*7:9*/2)
  /*6:19*/(/*4:16*/456 /*4:16*/456)
]
)");

  EXPECT_EQ(transform.environmentDepth(), 1);

  EXPECT_EQ_GDVSER(transform.innermostARVariableNames(),
    (std::set<GDVSymbol>{
      "x"_sym,
    }));

  EXPECT_EQ_GDVSER(transform.innermostARFunctionNames(),
    (std::set<GDVSymbol>{
      "f"_sym,
    }));

  GDValue actual2 = fnApplyTransformGDValue(commands);
  EXPECT_EQ(actual2, actual);
}


void test_fileLocs()
{
  GDValue defns = fromGDVN_asIfFile("defns", R"(
    [
      Define(x 3)
      Define(square(x) times(x x))
    ]
  )");
  EXPECT_EQ(defns.dumpToString(),
R"(/*defns:2:5*/[
  /*defns:3:7*/Define(/*defns:3:14*/x /*defns:3:16*/3)
  /*defns:4:7*/Define(
    /*defns:4:14*/square(/*defns:4:21*/x)
    /*defns:4:24*/times(/*defns:4:30*/x /*defns:4:32*/x)
  )
]
)");

  GDValue uses = fromGDVN_asIfFile("uses", R"(
    [
      7
      x
      square(5)
    ]
  )");
  EXPECT_EQ(uses.dumpToString(),
R"(/*uses:2:5*/[
  /*uses:3:7*/7
  /*uses:4:7*/x
  /*uses:5:7*/square(/*uses:5:14*/5)
]
)");

  GDValueFnApplyTransform transform;
  transform.selfCheck();

  // Put the definitions into the environment.
  transform.transform(defns);
  transform.selfCheck();

  // Use them.
  GDValue actual = transform.transform(uses);

  // The main point of this test is that this result has a mixture of
  // locations based on where each value was originally written.  The
  // ability to do this is why my original idea of not storing file
  // locations did not work.
  EXPECT_EQ(actual.dumpToString(),
R"(/*uses:2:5*/[
  /*uses:3:7*/7
  /*defns:3:16*/3
  /*defns:4:24*/times(/*uses:5:14*/5 /*uses:5:14*/5)
]
)");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_fnapply_transform()
{
  test_basics();
  test_fileLocs();
}


// EOF
