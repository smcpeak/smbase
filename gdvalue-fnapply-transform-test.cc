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


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_fnapply_transform()
{
  test_basics();
}


// EOF
