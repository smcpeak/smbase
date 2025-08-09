// buffered-streambuf.cc
// Code for `buffered-streambuf` module.

#include "buffered-streambuf.h"        // this module

#include "smbase/chained-cond.h"       // smbase::cc::{le_le, z_le_le}
#include "smbase/exc.h"                // GENERIC_CATCH_BEGIN,END
#include "smbase/overflow.h"           // convertNumber
#include "smbase/xassert.h"            // xassert, xassertPrecondition

#include <cstddef>                     // std::size_t
#include <cstring>                     // std::memmove

using namespace smbase;


void BufferedStreambuf::setCurrentOffset(std::size_t offset)
{
  xassertPrecondition(cc::z_le_le(offset, m_buffer.size()));

  // This sets all three pointers.  (There isn't a direct way to just
  // set the current pointer alone.)
  setp(m_buffer.data(), m_buffer.data() + m_buffer.size());

  // Now adjust the current pointer.
  pbump(offset);

  xassert(pptr() == pbase() + offset);
}


bool BufferedStreambuf::flushBuffer()
{
  std::streamsize const numToWrite =
    convertNumber<std::streamsize>(pptr() - pbase());

  std::streamsize const numWritten =
    writeAllToDestination(pbase(), numToWrite);

  if (numWritten < numToWrite) {
    // Move the data we could not write to the start of the buffer so if
    // the issue with the destination is resolved, we'll be ready.
    std::memmove(pbase(),
                 pbase() + numWritten,
                 numToWrite - numWritten);

    // Set the current pointer to be right after the unsent data.
    setCurrentOffset(numToWrite - numWritten);

    return false;
  }

  else {
    // All data was sent.
    setCurrentOffset(0);
    return true;
  }
}


auto BufferedStreambuf::overflow(int_type ch) -> int_type
{
  // First, flush the buffer.
  if (!flushBuffer()) {
    return traits_type::eof();
  }

  // Now add `ch` to it.
  if (ch != traits_type::eof()) {
    *(pptr()) = static_cast<char>(ch);
    pbump(1);
    return ch;
  }

  else {
    // Calling `overflow(eof())` is evidently one way to request a
    // flush.  But since returning `eof()` signals failure, we need to
    // return something else.
    //
    // (This is a really weird API design.)
    return traits_type::not_eof(ch);
  }
}


std::streamsize BufferedStreambuf::xsputn(
  const char *src, std::streamsize count)
{
  xassertPrecondition(count >= 0);

  if (static_cast<std::size_t>(count) > m_buffer.size()) {
    // Amount exceeds one buffer.  First, flush.
    if (!flushBuffer()) {
      return 0;
    }

    // When write as much as we can directly.
    return writeAllToDestination(src, count);
  }

  // Available space in the buffer.
  std::streamsize availSpace =
    convertNumber<std::streamsize>(epptr() - pptr());

  // Flush immediately if no space.
  if (availSpace == 0) {
    if (!flushBuffer()) {
      return 0;
    }
  }

  // Now we should have enough space for all of the data.
  availSpace = convertNumber<std::streamsize>(epptr() - pptr());
  xassert(availSpace > 0);
  xassert(count <= availSpace);

  // Append it to the buffer.
  std::memcpy(pptr(), src, count);
  pbump(convertNumber<int>(count));
  return count;
}


int BufferedStreambuf::sync()
{
  return flushBuffer()? 0 : -1;
}


std::streamsize BufferedStreambuf::writeAllToDestination(
  char const *src, std::streamsize totalToWrite)
{
  xassertPrecondition(totalToWrite >= 0);

  std::streamsize totalWritten = 0;

  while (totalWritten < totalToWrite) {
    std::streamsize written;
    try {
      written = writeToDestination(src + totalWritten,
                                   totalToWrite - totalWritten);
    }
    catch (std::exception &x) {
      if (!m_exceptionMessage) {
        m_exceptionMessage = x.what();
      }

      // Treat like `written == 0`.
      break;
    }

    xassert(cc::z_le_le(written, totalToWrite - totalWritten));

    if (written == 0) {
      break;
    }

    totalWritten += written;
  }

  xassert(cc::z_le_le(totalWritten, totalToWrite));
  return totalWritten;
}


void BufferedStreambuf::autoflush() noexcept
{
  GENERIC_CATCH_BEGIN

  if (!m_exceptionMessage) {
    // Flush, ignoring the return value and sending any exception to
    // `smbase::printUnhandled`.
    sync();
  }

  GENERIC_CATCH_END
}


BufferedStreambuf::~BufferedStreambuf() noexcept
{
  // Do *not* flush.  We no longer have access to the subclass
  // `writeToDestination` method nor any of the data it would have used.
}


BufferedStreambuf::BufferedStreambuf(std::size_t bufSize)
  : m_buffer(bufSize),
    m_exceptionMessage()
{
  xassertPrecondition(bufSize > 0);

  setCurrentOffset(0);

  selfCheck();
}


void BufferedStreambuf::selfCheck() const
{
  xassert(cc::le_le(pbase(), pptr(), epptr()));
}


// EOF
