// counting-ostream.h
// ostream that just counts the characters written to it.

/* Loosely based on answers found at:

     https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
     https://stackoverflow.com/questions/41377045/is-there-a-simple-way-to-get-the-number-of-characters-printed-in-c
     https://stackoverflow.com/questions/27534955/c-get-number-of-characters-printed-when-using-ofstream

   with the first one providing the actual code starting point.

   I subsequently submitted my first revision to Stack Exchange Code
   Review:

     https://codereview.stackexchange.com/questions/292109/ostream-that-counts-and-discards-the-characters-written-to-it

   and the current version reflects some of that feedback.
*/


#ifndef COUNTING_OSTREAM_H
#define COUNTING_OSTREAM_H

#include <cstddef>                     // std::size_t
#include <iostream>                    // std::streambuf, std::ostream


// Stream buffer that merely counts and discards all characters written.
class CountingStreambuf : public std::streambuf {
public:      // data
  // Count of characters seen.
  std::size_t m_count;

protected:   // methods
  // Override 'overflow' to count and discard written characters.
  virtual int_type overflow(int_type /*c*/) override
  {
    ++m_count;

    // Anything other than Traits::eof() (usually -1) means success.
    return 0;
  }

  // Also override 'xsputn' to handle large writes more efficiently.
  virtual std::streamsize xsputn(
    char_type const * /*s*/,
    std::streamsize count) override
  {
    m_count += count;

    // Returns the number of characters successfully written.
    return count;
  }

public:      // methods
  CountingStreambuf()
    : m_count(0)
  {}
};


// Present the 'std::ostream' interface and count the number of
// characters written to it.  That is, the final count equals what one
// would get by instead writing to a 'std::ostringstream' and then
// asking for the length of the string it contains at the end.  But this
// class avoids the bulk of the memory allocation that that would
// entail.  (Some formatting operations, like writing an integer, may do
// a small amount of local allocation anyway.)
class CountingOStream : public std::ostream {
private:     // data
  // The buffer to which the formatted data is sent.
  CountingStreambuf m_streambuf;

public:      // methods
  /* Build a stream to count and discard characters.

     In the member initializer list, we cannot just say
     'std::ostream(&m_streambuf)' because that has undefined behavior
     because the member has not yet been constructed.  Thus, we
     construct with nullptr and then re-initialize after the member is
     constructed.

     Constructing with nullptr sets badbit, which is a little scary, but
     calling 'init' with a non-null pointer clears that.

     Note: GCC libc++ has a bug in that it has a protected default
     constructor for std::ostream (which just does 'init(0)'), but no
     version of the C++ standard has that.  MSVC libc++ also does not
     have such a constructor.  The constructor that accepts a pointer is
     standard, so that is what we call.
  */
  CountingOStream()
    : std::ostream(nullptr),
      m_streambuf()
  {
    init(&m_streambuf);
  }

  // Get/set the count.
  std::size_t getCount() const
    { return m_streambuf.m_count; }
  void setCount(std::size_t c)
    { m_streambuf.m_count = c; }
};


#endif // COUNTING_OSTREAM_H
