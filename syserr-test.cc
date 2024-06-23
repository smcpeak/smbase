// syserr-test.cc
// Tests for syserr.

// This file is in the public domain.

#include "syserr.h"                    // module under test

#include "container-util.h"            // contains
#include "nonport.h"                   // changeDirectory
#include "set-util.h"                  // setMapElements

#include <cstdlib>                     // std::exit
#include <functional>                  // std::function

using namespace smbase;


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
                   std::set<XSysError::Reason> const &reasons)
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
  catch (XSysError &x) {
    if (!contains(reasons, x.reason)) {
      // Convert the expected reasons into strings.
      std::set<std::string> reasonStrings =
        setMapElements<std::string>(reasons,
          [](XSysError::Reason r) -> std::string {
            return XSysError::getReasonString(r);
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
           XSysError::R_PATH_NOT_FOUND,
           XSysError::R_FILE_NOT_FOUND);

  TRY_FAIL(createDirectory("test"),
           XSysError::R_ALREADY_EXISTS);

  TRY_FAIL(isDirectory("doesnt.exist"),
           XSysError::R_FILE_NOT_FOUND);

  if (errors > 0) {
    cout << errors << " error(s)\n";
    std::exit(2);
  }
}


// EOF
