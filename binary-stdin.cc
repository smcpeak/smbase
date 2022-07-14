// binary-stdin.cc
// Code for binary-stdin.h.

// smbase
#include "sm-platform.h"               // PLATFORM_IS_WINDOWS
#include "syserr.h"                    // xsyserror

// POSIX
#include <unistd.h>                    // STDIN_FILENO, etc.

// Windows
#ifdef __WIN32__
  #include <fcntl.h>                   // _O_BINARY
  #include <io.h>                      // _setmode
#else
  // Dummy definitions so the inactive code can compile.
  static int _setmode(int fd, int mode) {}
  #define _O_BINARY 0
#endif


void setFileDescriptorToBinary(int fd)
{
  if (PLATFORM_IS_WINDOWS) {
    if (_setmode(fd, _O_BINARY) < 0) {
      xsyserror("_setmode");
    }
  }
}


void setStdinToBinary()
{
  setFileDescriptorToBinary(STDIN_FILENO);
}


void setStdoutToBinary()
{
  setFileDescriptorToBinary(STDOUT_FILENO);
}


void setStderrToBinary()
{
  setFileDescriptorToBinary(STDERR_FILENO);
}


// EOF
