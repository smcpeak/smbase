#!/usr/bin/env python3
"""
Run a program and compare its output to what is expected.

Before comparison, the output will first be normalized according to two
rules:

1. If the string "VOLATILE" appears anywhere, the entire line is
replaced with "VOLATILE".

2. Within a line, if "0x" is followed by two or more hexadecimal digits,
then the entire sequence will be replaced by "0xHEXDIGITS" unless the
digits match "7F+", meaning they denote -1 as a signed twos-complement
integer (in which case they are not replaced).
"""

import argparse              # argparse
import difflib               # difflib.unified_diff
import os                    # os.getenv
import re                    # re.compile
import signal                # signal.signal
import subprocess            # subprocess.run
import sys                   # sys.argv, sys.stderr
import time                  # time.sleep
import traceback             # traceback.print_exc


# -------------- BEGIN: boilerplate -------------
# These are things I add at the start of every Python program to
# allow better error reporting.

# Positive if debug is enabled, with higher values enabling more printing.
debugLevel = 0
if (os.getenv("DEBUG")):
  debugLevel = int(os.getenv("DEBUG"))

def debugPrint(str):
  """Debug printout when DEBUG >= 2."""
  if debugLevel >= 2:
    print(str)

# Ctrl-C: interrupt the interpreter instead of raising an exception.
signal.signal(signal.SIGINT, signal.SIG_DFL)

class Error(Exception):
  """A condition to be treated as an error."""
  pass

def die(message):
  """Throw a fatal Error with message."""
  raise Error(message)

def exceptionMessage(e):
  """Turn exception 'e' into a human-readable message."""
  t = type(e).__name__
  s = str(e)
  if s:
    return f"{t}: {s}"
  else:
    return f"{t}"

def call_main():
  """Call main() and catch exceptions."""
  try:
    main()

  except SystemExit as e:
    raise      # Let this one go, otherwise sys.exit gets "caught".

  except BaseException as e:
    print(f"{exceptionMessage(e)}", file=sys.stderr)
    if (debugLevel >= 1):
      traceback.print_exc(file=sys.stderr)
    sys.exit(2)
# --------------- END: boilerplate --------------

def splitLines(data):
  """Decode byte list 'data' to text, split it at line boundaries
  (tolerating both LF and CRLF), and return the list of lines.  If the
  last line does not end with a newline, add another line indicating
  that fact."""

  text = data.decode()
  lines = text.splitlines()
  if len(text) > 0 and text[len(text)-1] != "\n":
    lines += ["\\ no newline"]
  return lines


def writeLinesToFile(lines, fname):
  """Write 'lines', which has no newlines, to 'fname' with LF line endings."""

  with open(fname, "w", newline="\n") as f:
    for line in lines:
      print(line, file=f)


# Hexadecimal numbers with at least two digits.
hexDigitsRE = re.compile(r"0x[0-9a-fA-F][0-9a-fA-F]+")

# Particular hex sequences that should not be replaced.  The ending '$'
# is important because the *entire* sequence of digits needs to match,
# not just a prefix.
nonReplaceHexRE = re.compile(r"0x7F+$");

def hexReplacer(m):
  """What to replace a match of 'hexDigitsRE' with."""

  if nonReplaceHexRE.match(m.group(0)):
    return m.group(0)
  else:
    return "0xHEXDIGITS"


# If "VOLATILE" appears on a line, treat the entire line as volatile
# in the sense of changing from run to run, and hence should not be
# included in the normalized output.
volatileRE = re.compile(r"VOLATILE")

def normalizeOutput(line):
  """Remove strings that vary from run to run so the result is suitable
  as expected test output."""

  if volatileRE.search(line):
    return "VOLATILE"
  else:
    line = hexDigitsRE.sub(hexReplacer, line)
  return line


def readLinesNoNL(file):
  """Like file.readlines(), but strip newlines."""
  return [line.rstrip("\n") for line in file.readlines()]


def main():
  # Parse command line.
  parser = argparse.ArgumentParser()
  parser.add_argument("--actual",
    help="If provided, write the actual output to this file.")
  parser.add_argument("--expect", required=True,
    help="Expected output.")
  parser.add_argument("--argfile",
    help="Run the program once per line in ARGFILE.")
  parser.add_argument("program",
    help="Program to run.")
  parser.add_argument("progArgs", nargs=argparse.REMAINDER,
    help="Arguments to the program.")
  opts = parser.parse_args()

  # Read the expected output.
  with open(opts.expect) as f:
    expectLines = readLinesNoNL(f)

  # List of extra argument lines.
  extraArgumentLines = [""]
  if opts.argfile:
    with open(opts.argfile) as f:
      extraArgumentLines = readLinesNoNL(f)

  # List of accumulated output lines.
  actualLines = []

  # Run the program, possibly more than once.
  for extraArgumentLine in extraArgumentLines:
    # Form the command to run, splitting extra arguments at whitespace.
    command = [opts.program] + opts.progArgs + extraArgumentLine.split()

    # If we are going to run the program multiple times, print a header
    # saying which one this is.
    if opts.argfile:
      actualLines += [f"======== {' '.join(command)} ========"]

    # Run the program, capturing output.
    proc = subprocess.run(command, capture_output=True);

    # Combine the stdout, stderr, and exit code into one list.
    actualLines += ["---- stdout ----"]
    actualLines += splitLines(proc.stdout)
    actualLines += ["---- stderr ----"]
    actualLines += splitLines(proc.stderr)
    actualLines += ["---- exit status ----"]
    actualLines += [f"Exit {proc.returncode}"]

  # Normalize it.
  actualLines = [normalizeOutput(line) for line in actualLines]

  # Optionally save it.
  if opts.actual:
    writeLinesToFile(actualLines, opts.actual)

  diff = list(difflib.unified_diff(expectLines, actualLines,
    fromfile="expect", tofile="actual", lineterm=""))
  if len(diff) > 0:
    print("Results are different:")
    for diffLine in diff:
      print(diffLine)

    makeflags = os.getenv("MAKEFLAGS")

    update = os.getenv("UPDATE_EXPECT")
    if update == "1":
      print(f"UPDATE_EXPECT is 1.  Updating {opts.expect} with the new output.")
      writeLinesToFile(actualLines, opts.expect)

    elif makeflags != None and "-j" in makeflags:
      print("Since MAKEFLAGS contains '-j', I will not prompt to update.")
      sys.exit(2)

    elif update == "prompt" or update == "promptyes":
      answer = "x"
      while (answer != "y" and answer != "n" and
             not (answer == "" and update == "promptyes")):
        if update == "prompt":
          answer = input("Update expected output (y/n)? ")
        else:
          answer = input("Update expected output (Y/n)? ")
      if answer != "n":
        print(f"Answer was 'y'.  Updating {opts.expect} with the new output.")
        writeLinesToFile(actualLines, opts.expect)
      else:
        print(f"Answer was 'n'.  Exiting with error.");
        sys.exit(2)

    else:
      print("Re-run with UPDATE_EXPECT=1 or UPDATE_EXPECT=prompt[yes] to update.")
      sys.exit(2)


call_main()


# EOF
