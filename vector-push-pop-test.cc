// vector-push-pop-test.cc
// Tests for `vector-push-pop.h`.

// This file is in the public domain.

#include "vector-push-pop.h"           // module under test

// This must come before `sm-test.h` so `EXPECT_EQ` can print vectors.
#include "vector-util.h"               // operator<<(std::vector)

#include "dev-warning.h"               // g_devWarningHandler
#include "save-restore.h"              // SET_RESTORE
#include "sm-macros.h"                 // OPEN_ANONYMOUS_NAMESPACE
#include "sm-test.h"                   // EXPECT_EQ, verbose

#include <optional>                    // std::optional
#include <stdexcept>                   // std::runtime_error
#include <string>                      // std::string
#include <utility>                     // std::move
#include <vector>                      // std::vector

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


using IntVec = std::vector<int>;
using StringVec = std::vector<std::string>;


// ---------------------- Capturing dev warnings -----------------------
// Number of dev warnings seen by `captureDevWarning`.
int s_devWarningCount = 0;

// Message of the most recent captured dev warning.
std::string s_lastDevWarning;


void captureDevWarning(char const *file, int line, char const *msg)
{
  ++s_devWarningCount;
  s_lastDevWarning = msg;
}


// Within the scope of this macro, dev warnings are captured rather than
// printed or aborting, and the capture state starts out clear.
#define CAPTURE_DEV_WARNINGS()                       \
  SET_RESTORE(g_devWarningHandler, &captureDevWarning); \
  s_devWarningCount = 0;                             \
  s_lastDevWarning.clear() /* user ; */


// --------------------------- VectorPushPop ---------------------------
void test_pushPop_lvalue()
{
  IntVec vec{1};

  {
    int value = 2;
    VectorPushPop<int> vpp(vec, value);
    EXPECT_EQ(vec, (IntVec{1, 2}));
    EXPECT_EQ(&vpp.m_vec, &vec);

    // The value was copied, so changing the source has no effect.
    value = 3;
    EXPECT_EQ(vec, (IntVec{1, 2}));
  }

  EXPECT_EQ(vec, (IntVec{1}));
}


void test_pushPop_rvalue()
{
  StringVec vec;

  // Long enough to not fit in the small-string buffer, so we can tell
  // that the buffer was transferred rather than copied.
  std::string value(100, 'x');
  char const *origData = value.data();

  {
    VectorPushPop<std::string> vpp(vec, std::move(value));
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec.back(), std::string(100, 'x'));
    EXPECT_TRUE(vec.back().data() == origData);
  }

  EXPECT_TRUE(vec.empty());
}


void test_pushPop_nested()
{
  IntVec vec;

  {
    VectorPushPop<int> vpp1(vec, 1);
    EXPECT_EQ(vec, (IntVec{1}));

    {
      VectorPushPop<int> vpp2(vec, 2);
      VectorPushPop<int> vpp3(vec, 3);
      EXPECT_EQ(vec, (IntVec{1, 2, 3}));
    }

    EXPECT_EQ(vec, (IntVec{1}));
  }

  EXPECT_TRUE(vec.empty());
}


void test_pushPop_exception()
{
  IntVec vec{1};

  try {
    VectorPushPop<int> vpp(vec, 2);
    EXPECT_EQ(vec, (IntVec{1, 2}));
    throw std::runtime_error("unwind");
  }
  catch (std::runtime_error &) {
    // The pop happened during unwinding.
    EXPECT_EQ(vec, (IntVec{1}));
  }

  EXPECT_EQ(vec, (IntVec{1}));
}


void test_pushPop_emptyAtPop()
{
  CAPTURE_DEV_WARNINGS();

  IntVec vec;
  {
    VectorPushPop<int> vpp(vec, 1);
    EXPECT_EQ(s_devWarningCount, 0);

    // Remove the pushed element out from under `vpp`.
    vec.clear();
  }

  EXPECT_EQ(s_devWarningCount, 1);
  EXPECT_EQ(s_lastDevWarning, "vector to pop is empty");
  EXPECT_TRUE(vec.empty());
}


void test_VECTOR_PUSH_POP()
{
  IntVec vec;

  {
    int two = 2;

    // Multiple uses in one scope do not collide.
    VECTOR_PUSH_POP(vec, 1);
    VECTOR_PUSH_POP(vec, two);
    EXPECT_EQ(vec, (IntVec{1, 2}));

    // Conversion in argument.
    VECTOR_PUSH_POP(vec, 3.1);
    EXPECT_EQ(vec, (IntVec{1, 2, 3}));
  }

  EXPECT_TRUE(vec.empty());
}


// ------------------------- VectorPushPopOpt --------------------------
void test_pushPopOpt_absent()
{
  CAPTURE_DEV_WARNINGS();

  IntVec vec{1};

  // lvalue
  {
    std::optional<int> none;
    VectorPushPopOpt<int> vpp(vec, none);
    EXPECT_FALSE(vpp.m_doPushPop);
    EXPECT_EQ(vec, (IntVec{1}));
  }
  EXPECT_EQ(vec, (IntVec{1}));

  // rvalue
  {
    VectorPushPopOpt<int> vpp(vec, std::optional<int>());
    EXPECT_FALSE(vpp.m_doPushPop);
    EXPECT_EQ(vec, (IntVec{1}));
  }
  EXPECT_EQ(vec, (IntVec{1}));

  // With nothing pushed, an empty vector at scope exit is fine.
  IntVec empty;
  {
    VectorPushPopOpt<int> vpp(empty, std::optional<int>());
  }
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(s_devWarningCount, 0);
}


void test_pushPopOpt_lvalue()
{
  IntVec vec{1};

  {
    std::optional<int> two(2);
    VectorPushPopOpt<int> vpp(vec, two);
    EXPECT_TRUE(vpp.m_doPushPop);
    EXPECT_EQ(vec, (IntVec{1, 2}));

    // The source still has its value.
    EXPECT_TRUE(two.has_value());
    EXPECT_EQ(*two, 2);
  }

  EXPECT_EQ(vec, (IntVec{1}));
}


void test_pushPopOpt_rvalue()
{
  StringVec vec;

  std::optional<std::string> value(std::string(100, 'x'));
  char const *origData = value->data();

  {
    VectorPushPopOpt<std::string> vpp(vec, std::move(value));
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec.back(), std::string(100, 'x'));
    EXPECT_TRUE(vec.back().data() == origData);
  }

  EXPECT_TRUE(vec.empty());
}


void test_pushPopOpt_inhibitPop()
{
  IntVec vec{1};

  {
    VectorPushPopOpt<int> vpp(vec, std::optional<int>(2));
    EXPECT_EQ(vec, (IntVec{1, 2}));

    // Client decides to keep the pushed element.
    vpp.m_doPushPop = false;
  }

  EXPECT_EQ(vec, (IntVec{1, 2}));
}


void test_pushPopOpt_exception()
{
  IntVec vec;

  try {
    VectorPushPopOpt<int> vpp(vec, std::optional<int>(5));
    EXPECT_EQ(vec, (IntVec{5}));
    throw std::runtime_error("unwind");
  }
  catch (std::runtime_error &) {
    EXPECT_TRUE(vec.empty());
  }
}


void test_pushPopOpt_emptyAtPop()
{
  CAPTURE_DEV_WARNINGS();

  IntVec vec;
  {
    VectorPushPopOpt<int> vpp(vec, std::optional<int>(1));
    vec.clear();
  }

  EXPECT_EQ(s_devWarningCount, 1);
  EXPECT_EQ(s_lastDevWarning, "vector to pop is empty");
  EXPECT_TRUE(vec.empty());
}


void test_VECTOR_PUSH_POP_OPT()
{
  IntVec vec;

  {
    std::optional<int> one(1);
    std::optional<int> none;

    VECTOR_PUSH_POP_OPT(vec, one);
    VECTOR_PUSH_POP_OPT(vec, none);
    VECTOR_PUSH_POP_OPT(vec, std::optional<int>(3));
    EXPECT_EQ(vec, (IntVec{1, 3}));
  }

  EXPECT_TRUE(vec.empty());
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_vector_push_pop()
{
  test_pushPop_lvalue();
  test_pushPop_rvalue();
  test_pushPop_nested();
  test_pushPop_exception();
  test_pushPop_emptyAtPop();
  test_VECTOR_PUSH_POP();

  test_pushPopOpt_absent();
  test_pushPopOpt_lvalue();
  test_pushPopOpt_rvalue();
  test_pushPopOpt_inhibitPop();
  test_pushPopOpt_exception();
  test_pushPopOpt_emptyAtPop();
  test_VECTOR_PUSH_POP_OPT();
}


// EOF
