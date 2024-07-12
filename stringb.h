// stringb.h
// Define the 'stringb' macro that allows in-place string construction
// using 'operator<<'.

#ifndef SMBASE_STRINGB_H
#define SMBASE_STRINGB_H

// IWYU pragma: begin_exports
#include <string>                      // std::string
#include <sstream>                     // std::ostringstream
// IWYU pragma: end_exports


/* Convert an rvalue reference to `ostream` into an lvalue reference to
   the same object.

   Why is this allowed?  I don't really know.  The compiler will not
   just let me explicitly cast one to the other, but is happy to let the
   conversion happen implicitly this way.

   Why is this necessary?  For inscrutible reasons, without this, if I
   try to insert a `std::vector` by using the `operator<<` defined in
   `vector-util.h`, overload resolution fails and the compiler spews
   over 1000 lines of error messages.  But if I use this function to
   wrap the temporary, then it works.

   C++, amiright?
*/
inline std::ostream &ostreamR2LReference(std::ostream &&os)
{
  return os;
}

// Construct a string in-place using ostream operators.
#define stringb(stuff)                                                   \
  (                                                                      \
    /* 4. After all insertions are done, convert the stream reference */ \
    /* back to `ostringstream` so we can extract the string. */          \
    static_cast<std::ostringstream const &>(                             \
      /* 2. Convert the rvalue reference to an lvalue reference. */      \
      ostreamR2LReference(                                               \
        /* 1. Make a temporary `ostringstream`. */                       \
        std::ostringstream()                                             \
      /* 3. Insert arbitrary things into the stream. */                  \
      ) << stuff                                                         \
    /* 5. Extract the `std::string` result. */                           \
    ).str()                                                              \
  )

// Do the same but yield a 'char const *' to the temporary string
// object--this obviously should not be used after the termination of
// the originating full expression!
#define stringbc(stuff) (stringb(stuff).c_str())


#endif // SMBASE_STRINGB_H
