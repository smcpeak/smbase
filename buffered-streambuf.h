// buffered-streambuf.h
// Subclass of `std::ostream` that implements buffering only.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_BUFFERED_STREAMBUF_H
#define SMBASE_BUFFERED_STREAMBUF_H

#include "buffered-streambuf-fwd.h"    // fwds for this module

#include "smbase/sm-macros.h"          // NO_OBJECT_COPIES, OPEN_NAMESPACE

#include <cstddef>                     // std::size_t
#include <optional>                    // std::optional
#include <streambuf>                   // std::streambuf
#include <string>                      // std::string
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


// Output stream buffer that just manages the buffering.  It has a pure
// virtual function, `writeToDestination`, to perform writes to the
// actual destination (e.g., file).
class BufferedStreambuf : public std::streambuf {
  NO_OBJECT_COPIES(BufferedStreambuf);

private:     // data
  /* Buffer to hold data before it goes to `WriteFile`.

     In the streambuf API terminology:

       * `m_buffer.data()` is the "beginning pointer", also known as the
         "start of the put area".  It is accessible as `pbase().`

       * `m_buffer.data() + m_buffer.size()` is the "end pointer", also
         known as the "end of the put area".  It is accessible as
         `epptr()`.

     The "next pointer", a.k.a the "current put area", is maintained in
     a data member of the base class, and accessible as `pptr()`.

     Invariant: pbase() <= pptr() <= epptr()

     Notably, this means there might not be any room in the buffer when
     a method is invoked from the outside.
  */
  std::vector<char> m_buffer;

public:      // data
  /* If `writeToDestination` throws an exception, we catch it and store
     its `what()` here.  It is then up to the client to collect it
     and/or clear it.

     While a message is stored here, `autoflush` will not do anything.
     That way we don't re-trigger the same exception during the
     destructor.  Additionally, any additional exceptions from
     `writeToDestination` will be discarded.

     I would prefer a design that allowed the exception to propagate as
     a structured object, but at least in GCC's C++ library,
     `std::__ostream_insert`, which is used by all of the `ostream`
     write operations, swallows all exceptions, turning them into
     `badbit`.
  */
  std::optional<std::string> m_exceptionMessage;

  // If false, `autoflush` will not do anything.  If true, it will try
  // to flush, subject to the condition described for
  // `m_exceptionMessage`.  Initially true, and entirely under the
  // control of the client.
  bool m_enableAutoflush;

private:     // methods
  /* Set the beginning, current, and end pointers so that the current
     pointer is `offset` more than the beginning, i.e., there are
     `current` bytes in the buffer, ready to be sent.

     Requires: 0 <= offset <= m_buffer.size()

     Ensures: pptr() == pbase() + offset
  */
  void setCurrentOffset(std::size_t offset);

  /* Flush all of the buffered data, returning true on success.

     This can be called when the buffer is already empty, in which case
     it will just return true.

     On error, return false or throw an exception; the latter happens
     only if `writeToDestination` throws.

     Ensures: if return, then pptr() == pbase()
  */
  bool flushBuffer();

protected:   // methods
  /* If `ch` is not `eof()`, conceptually append it to `m_buffer` (but
     possibly not actually, since the buffer could be full).  Then
     send at least one character of the possibly conceptually expanded
     `m_buffer` to the destination.

     Returns `eof()` on failure, any other value on success.
  */
  virtual int_type overflow(int_type ch) override;

  // Add many characters to the output.  Returns the number successfully
  // written, which may be zero in case of error.
  //
  // Requires: count >= 0
  virtual std::streamsize xsputn(
    const char *src, std::streamsize count) override;

  // Flush output, returning 0 for success and -1 for failure.
  //
  // However, this will throw if `writeToDestination` does.
  virtual int sync() override;

  /* Write `count` characters from `src` to the destination.  Return the
     number actually written, which may be less than `count`; that is
     not an error case.

     This must block until at least one character is successfully
     written (or an error occurs).

     On error, this function can either return 0 or throw an exception.
     However, an exception will not propagate out of this class, instead
     getting turned into `m_exceptionMessage` and, typically, an ostream
     `badbit`.

     Requires: count >= 0

     Requires: All of [src, src+count-1] is valid.

     Ensures: 0 <= return <= count
  */
  virtual std::streamsize writeToDestination(
    const char *src, std::streamsize count) = 0;

  /* Write `count` bytes from `src` to the destination by repeatedly
     (if necessary) calling `writeToDestination`.  If at some point that
     function returns 0 (meaning error), return a short count.
     Otherwise, return `count`.

     Requires: count >= 0

     Requires: All of [src, src+count-1] is valid.

     Ensures: 0 <= return <= count
  */
  std::streamsize writeAllToDestination(
    char const *src, std::streamsize count);

  // If `m_exceptionMessage` is set, do nothing.  Otherwise, call
  // `sync`, but pass any exceptions to `smbase::printUnhandled` before
  // continuing normally.  This is meant to be called from a subclass
  // destructor.
  void autoflush() noexcept;

public:      // methods
  // This does *not* flush the stream because, by the time this dtor
  // runs, the vtable pointer for `writeToDestination` points at the
  // implementation in this class.  A subclass implementation can call
  // `autoflush()` to flush.
  virtual ~BufferedStreambuf() noexcept override;

  // Create with a buffer of the specified size.
  //
  // Requires: bufSize > 0
  explicit BufferedStreambuf(std::size_t bufSize = 0x1000);

  // Assert invariants.
  void selfCheck() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_BUFFERED_STREAMBUF_H
