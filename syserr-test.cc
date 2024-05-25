// syserr-test.cc
// Tests for syserr.

// This file is in the public domain.

#include "syserr.h"                    // module under test

#include "container-utils.h"           // contains
#include "nonport.h"                   // changeDirectory
#include "set-utils.h"                 // setMapElements

#include <cstdlib>                     // std::exit
#include <functional>                  // std::function


// Call `failingCall`, expecting that it will return false, meaning it
// failed.  It is also expected to set `errno` or the current OS
// equivalent.  We then check that the syserr Reason code is among those
// in `reasons`.
//
// Returns 1 if the check failed (the call did not fali, or failed with
// the wrong reason) and 0 for success.  This return value is meant to
// be added to the count of failed tests.
static int tryFail(std::function<bool ()> failingCall,
                   char const *failingCallText,
                   std::set<xSysError::Reason> const &reasons)
{
  try {
    if (failingCall()) {
      std::cout << "ERROR: " << failingCallText << " should have failed\n";
      return 1;
    }
    else {
      // `errno` should be set, so we can test it.
      xsyserror(failingCallText);
    }
  }
  catch (xSysError &x) {
    if (!contains(reasons, x.reason)) {
      // Convert the expected reasons into strings.
      std::set<std::string> reasonStrings =
        setMapElements<std::string>(reasons,
          [](xSysError::Reason r) -> std::string {
            return xSysError::getReasonString(r);
          });

      cout << "ERROR: " << failingCallText << " returned '"
           << x.reasonString << "' but one of "
           << reasonStrings << " was expected.\n";
      return 1;
    }
  }

  return 0;
}


// Called by unit-tests.cc.
void test_syserr()
{
  int errors = 0;

  #define TRY_FAIL(failingCall, ...)                \
    errors +=                                       \
      tryFail([]() -> bool { return failingCall; }, \
              #failingCall,                         \
              {__VA_ARGS__})


  TRY_FAIL(changeDirectory("some.strange.name/yadda"),
           xSysError::R_PATH_NOT_FOUND,
           xSysError::R_FILE_NOT_FOUND);

  TRY_FAIL(createDirectory("test"),
           xSysError::R_ALREADY_EXISTS);

  TRY_FAIL(isDirectory("doesnt.exist"),
           xSysError::R_FILE_NOT_FOUND);

  if (errors == 0) {
    cout << "success!\n";
  }
  else {
    cout << errors << " error(s)\n";
    std::exit(2);
  }
}


// EOF
