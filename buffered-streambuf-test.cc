// buffered-streambuf-test.cc
// Tests for `buffered-streambuf` module.

#include "smbase/buffered-streambuf.h" // module under test

#include "smbase/exc.h"                // smbase::xmessage
#include "smbase/gdvalue.h"            // gdv::toGDValue (for EXPECT_EQ_GDV)
#include "smbase/overflow.h"           // convertNumber
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE, smbase_loopi
#include "smbase/sm-test.h"            // EXPECT_EQ[_GDV], DIAG

#include <cstring>                     // std::strlen
#include <initializer_list>            // std::initializer_list
#include <iostream>                    // std::ostream
#include <optional>                    // std::optional

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// `BufferedStreambuf` that writes to a string.
class TestBufferedStreambuf : public BufferedStreambuf {
public:      // data
  // Writes append to this.
  std::string &m_output;

  // If set, then we can only write this much in a single call to
  // `writeToDestination`.  If zero, it simulates a non-throwing error.
  std::optional<std::streamsize> m_partialWriteLimit;

  // If set, then if this is zero, have `writeToDestination` report an
  // error.  If it is non-zero, decrement it.
  std::optional<int> m_errorCountdown;

  // If set, then when we try to report an error, throw `XMessage` with
  // this error string instead of returning 0.
  std::optional<std::string> m_errorString;

public:      // methods
  virtual ~TestBufferedStreambuf() override
  {
    autoflush();
  }

  TestBufferedStreambuf(std::size_t bufSize, std::string &output)
    : BufferedStreambuf(bufSize),
      m_output(output),
      m_partialWriteLimit(),
      m_errorCountdown(),
      m_errorString()
  {}

  virtual std::streamsize writeToDestination(
    char const *src, std::streamsize count) override
  {
    xassert(count >= 0);

    // Possibly impose the limit.
    if (m_partialWriteLimit && *m_partialWriteLimit < count) {
      count = *m_partialWriteLimit;
    }

    // Possibly error.
    if (m_errorCountdown) {
      if (*m_errorCountdown == 0) {
        if (m_errorString) {
          // Indicate error by exception.
          xmessage(*m_errorString);
        }
        else {
          // Indicate error by return value.
          return 0;
        }
      }

      else {
        --*m_errorCountdown;
      }
    }

    m_output.append(src, convertNumber<std::size_t>(count));
    return count;
  }

  // Assert that `m_buffer` matches `expect`.
  void expectBuffer(char const *expect)
  {
    // The buffer size is fixed.  This is the amount of meaningful data
    // in it.
    std::streamsize size = pptr() - pbase();
    EXPECT_EQ(size, std::strlen(expect));

    smbase_loopi(size) {
      EXN_CONTEXT_EXPR(i);
      EXPECT_EQ_NUMBERS(pbase()[i], expect[i]);
    }
  }

  // Call this protected member.
  int_type callOverflow(int ch)
  {
    return overflow(ch);
  }
};


// The usual setup.
#define TEST_SETUP(bufSize)                   \
  std::string output;                         \
  TestBufferedStreambuf buf(bufSize, output); \
  std::ostream os(&buf) /* user ; */


// Check that we have expected values for `output` and the internal
// buffer of `buf`.  I write them in this order since, logically, the
// eventual output consists of them concatenated in this order.
//
// I also check that the `ostream` has not seen any errors.
//
// TODO: Make an EXN_CONTEXT macro for the file/line combo.
#define EXPECT_OUT_BUF(expectOut, expectBuf)  \
  {                                           \
    EXN_CONTEXT(__FILE__ << ":" << __LINE__); \
    EXPECT_EQ_GDV(output, expectOut);         \
    {                                         \
      EXN_CONTEXT("buffer");                  \
      buf.expectBuffer(expectBuf);            \
    }                                         \
    EXPECT_EQ(os.fail(), false);              \
  }


void test_basics()
{
  TEST_SETUP(4);

  buf.expectBuffer("");

  os.put('A');
  EXPECT_OUT_BUF("", "A");

  os.put('B');
  EXPECT_OUT_BUF("", "AB");

  os.put('C');
  os.put('D');
  EXPECT_OUT_BUF("", "ABCD");

  // Trigger overflow.
  os.put('E');
  EXPECT_OUT_BUF("ABCD", "E");

  // One more.
  os.put('F');
  EXPECT_OUT_BUF("ABCD", "EF");

  // Flush all.
  os.flush();
  EXPECT_OUT_BUF("ABCDEF", "");
}


void test_smallXsputn()
{
  TEST_SETUP(10);

  os.write("Hello", 5);
  EXPECT_OUT_BUF("", "Hello");

  os.flush();
  EXPECT_OUT_BUF("Hello", "");
}


void test_largeXsputn()
{
  TEST_SETUP(4);

  std::string big(20, 'X');
  os.write(big.data(), big.size());

  EXPECT_OUT_BUF("XXXXXXXXXXXXXXXXXXXX", "");
}


void test_largeXsputnWithPreviousData()
{
  TEST_SETUP(4);

  os.put('A');
  EXPECT_OUT_BUF("", "A");

  // This will trigger an initial flush, then a direct write of all the
  // data passedhere.
  os.write("BCDEFGHIJ", 9);
  EXPECT_OUT_BUF("ABCDEFGHIJ", "");
}


void test_smallPartialWrite()
{
  TEST_SETUP(6);
  buf.m_partialWriteLimit = 3;

  // Internally, write in two steps.  Both fit.
  os.write("abcdef", 6);
  EXPECT_OUT_BUF("", "abcdef");

  os.flush();
  EXPECT_OUT_BUF("abcdef", "");
}


void test_largePartialWrite()
{
  TEST_SETUP(4);
  buf.m_partialWriteLimit = 3;

  // Internally, write in two steps.  Both will go directly to the
  // output since the total to write is larger than the buffer.
  os.write("abcdef", 6);
  EXPECT_OUT_BUF("abcdef", "");

  os.flush();
  EXPECT_OUT_BUF("abcdef", "");
}


// If I call this "test_overflow", it collides with the name of the
// entry point for testing the `overflow` module ...
void test_callOverflow()
{
  TEST_CASE("test_callOverflow");

  TEST_SETUP(4);

  EXPECT_EQ_NUMBERS(buf.callOverflow('a'), 'a');
  EXPECT_OUT_BUF("", "a");

  EXPECT_EQ_NUMBERS(buf.callOverflow('b'), 'b');
  EXPECT_OUT_BUF("a", "b");

  int const eof = BufferedStreambuf::traits_type::eof();
  int res = buf.callOverflow(eof);
  xassert(res != eof);    // `eof` would indicate failure.
  VPVAL(res);             // I see "0" here, FWIW.
  EXPECT_OUT_BUF("ab", "");

  // Doing it again should have no effect.
  res = buf.callOverflow(eof);
  xassert(res != eof);
  EXPECT_OUT_BUF("ab", "");
}


void test_failWriteImmediate()
{
  TEST_CASE("test_failWriteImmediate");

  TEST_SETUP(4);

  os << "hi";
  EXPECT_OUT_BUF("", "hi");

  // Force the write to fail.
  buf.m_partialWriteLimit = 0;
  os.flush();
  EXPECT_EQ(os.fail(), true);

  // The data should still be in the buffer.
  os.clear();
  EXPECT_OUT_BUF("", "hi");

  // We should be able to recover.
  buf.m_partialWriteLimit = std::nullopt;
  os.flush();
  EXPECT_OUT_BUF("hi", "");
}


void test_failWriteDelayed()
{
  TEST_CASE("test_failWriteDelayed");

  TEST_SETUP(10);

  buf.m_errorCountdown = 1;
  buf.m_partialWriteLimit = 2;

  os << "hello";
  EXPECT_OUT_BUF("", "hello");
  os.flush();
  EXPECT_EQ(os.fail(), true);

  // Hmmm, I guess this gets set too.
  EXPECT_EQ(os.bad(), true);

  // One write should have succeeded.
  os.clear();
  EXPECT_OUT_BUF("he", "llo");
}


void test_failExn()
{
  TEST_CASE("test_failExn");

  TEST_SETUP(10);

  buf.m_errorCountdown = 1;
  buf.m_partialWriteLimit = 2;
  buf.m_errorString = "this is the error";
  EXPECT_EQ(buf.m_exceptionMessage.has_value(), false);

  os << "hello";
  EXPECT_OUT_BUF("", "hello");

  // This will trigger an exception that will then be caught internally.
  os.flush();

  // It will leave the stream with the bad bit set.
  EXPECT_EQ(os.bad(), true);

  // And the message will be in `buf`.
  xassert(buf.m_exceptionMessage.has_value());
  VPVAL(*buf.m_exceptionMessage);
  EXPECT_HAS_SUBSTRING(*buf.m_exceptionMessage, "this is the error");

  // The write should have been split.
  EXPECT_EQ(output, "he");
  buf.expectBuffer("llo");

  // We now allow the destructor to run, which should *not* autoflush,
  // and hence not throw another exception.
}


// Demonstrate the autoflush capability (when an exception has not been
// caught).
void test_autoflush()
{
  std::string output;

  {
    TestBufferedStreambuf buf(10, output);
    std::ostream os(&buf);

    os << "hi";
    EXPECT_OUT_BUF("", "hi");
  }

  EXPECT_EQ(output, "hi");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_buffered_streambuf()
{
  test_basics();
  test_smallXsputn();
  test_largeXsputn();
  test_largeXsputnWithPreviousData();
  test_smallPartialWrite();
  test_largePartialWrite();
  test_callOverflow();
  test_failWriteImmediate();
  test_failWriteDelayed();
  test_failExn();
  test_autoflush();
}


// EOF
