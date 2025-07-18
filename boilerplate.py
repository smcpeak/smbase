# boilerplate.py
"""
Definitions to enhance error reporting in a Python program.

To use, first import all the symbols:

  from boilerplate import *

Then define `main` somewhere:

  main() -> None:
    ...

Finally, at the end, call it via `call_main`:

  if __name__ == "__main__":
    call_main(main)

Principally, this suppresses the stack trace on exceptions unless the
DEBUG envvar is set to a non-zero value.  Instead, it prints a single
line that includes the exception class name (since unfortunately some
exceptions rely on that to communicate the problem) and any text string
that accompanies it, falling back to the file:line where it was raised
if there is no text.

It also offers a few convenience functions for error reporting, such as
the `die` function.

Finally, it changes Ctrl+C so it terminates the script, similar to most
other programs, rather than raising an exception (which is often
ineffective at stopping the program).
"""

import os                    # getenv, path
import re                    # error
import signal                # signal
import sys                   # argv, stderr, stdin, exc_info
import traceback             # extract_tb, print_exc

from types import TracebackType
from typing import Callable, Optional, NoReturn, Tuple


# Positive if debug is enabled, with higher values enabling more printing.
debugLevel: int = 0
if debugEnvVal := os.getenv("DEBUG"):
  debugLevel = int(debugEnvVal)

def debugPrint(str: str) -> None:
  """Debug printout when DEBUG >= 2."""
  if debugLevel >= 2:
    print(str)


# Ctrl-C: interrupt the interpreter instead of raising an exception.
signal.signal(signal.SIGINT, signal.SIG_DFL)


class Error(Exception):
  """A condition to be treated as an error."""
  pass


def die(message: str) -> NoReturn:
  """Throw a fatal Error with message."""
  raise Error(message)


def findCallsiteForCompile(e: re.error) -> Optional[Tuple[str, int]]:
  """
  Given regex error `e`, search its traceback to find the likely
  location of the offending regex in the source code.  The algorithm
  looks for the first (outermost) frame for which the *next* frame is
  a call to `compile` in the file `re.py`.  It then returns the
  file name and line number of that frame.
  """

  tb: Optional[TracebackType] = e.__traceback__
  if tb is None:
    return None

  # Collect frames in order: outermost -> innermost.
  frames = []
  while tb is not None:
    frames.append(tb.tb_frame)
    tb = tb.tb_next

  # Now look for a frame where the next one is re.compile.
  for i in range(len(frames) - 1):
    next_frame = frames[i + 1]
    if (next_frame.f_code.co_name == "compile" and
        next_frame.f_code.co_filename.endswith("re.py")):
      # Return the outer frame's source location.
      outer_frame = frames[i]
      return (outer_frame.f_code.co_filename, outer_frame.f_lineno)

  return None


def exceptionMessage(e: BaseException) -> str:
  """Turn exception 'e' into a human-readable message."""

  # If the exception comes from another module, get its name followed
  # by a dot.  Otherwise get the empty string.
  m = type(e).__module__
  if m == "__main__" or m == "boilerplate" or m == "builtins":
    m = ""
  else:
    m += "."

  # The qualified name will include outer classes.
  t = m + type(e).__qualname__

  # Exception message.
  s = str(e)

  if s:
    if isinstance(e, re.error):
      # See if we can get a source location for a regex compile error.
      srcLoc = findCallsiteForCompile(e)
      if srcLoc is not None:
        file, line = srcLoc
        file = os.path.basename(file)
        s = f"{file}:{line}: {s}"

    return f"{t}: {s}"

  else:
    # If no extra info, try to get the location of the `raise`.  This is
    # needed for AssertionError, for example.
    tb: Optional[TracebackType] = e.__traceback__
    if tb:
      # Get the last frame.
      frame = traceback.extract_tb(tb)[-1]
      fname = os.path.basename(frame.filename)
      return f"{t} at {fname}:{frame.lineno}"

    else:
      # All we have is the exception class name.
      return t


def call_main(main: Callable[[], None]) -> None:
  """Call `main()` and catch exceptions."""
  try:
    main()

  except SystemExit as e:
    raise      # Let this one go, otherwise sys.exit gets "caught".

  except BaseException as e:
    print(f"{exceptionMessage(e)}", file=sys.stderr)
    if (debugLevel >= 1):
      traceback.print_exc(file=sys.stderr)
    sys.exit(2)


# EOF
