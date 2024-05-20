// stringb.h
// Define the 'stringb' macro that allows in-place string construction
// using 'operator<<'.

#ifndef SMBASE_STRINGB_H
#define SMBASE_STRINGB_H

#include <string>                      // std::string
#include <sstream>                     // std::ostringstream


// Construct a string in-place using ostream operators.
#define stringb(stuff) \
  (static_cast<std::ostringstream const &>(std::ostringstream() << stuff).str())

// Do the same but yield a 'char const *' to the temporary string
// object--this obviously should not be used after the termination of
// the originating full expression!
#define stringbc(stuff) (stringb(stuff).c_str())


#endif // SMBASE_STRINGB_H
