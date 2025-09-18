#!/usr/bin/env python3
"""
Check some ad-hoc coding rules on the input source files.

Rules:

  * There should not be any Carriage Return (0x0D) characters.

  * Header files should not have any "using namespace" at top level.

"""

import argparse              # argparse
import re                    # re.compile

from boilerplate import *
from typing import List


def readLines(filename: str) -> List[str]:
  """Get the lines in 'filename', including line terminators."""
  with open(filename, "rb") as file:
    ret: List[str] = []
    for line_bytes in file:
      ret.append(line_bytes.decode())
    return ret


# Number of violations detected.
numErrors: int = 0


def complain(fname: str, lineNumber: int, msg: str) -> None:
  """Report a violation."""

  print(f"{fname}:{lineNumber}: {msg}")

  global numErrors
  numErrors += 1


# Recognize the name of a C/C++ header file.
headerFnameRE = re.compile(r".*\.(h|hpp|hh|H)")

# "using namespace" unindented, meaning it is (probably) at top level.
usingNamespaceRE = re.compile(r"^using\s+namespace\b")


def checkFile(fname: str) -> None:
  """Check `fname`."""

  isHeader: bool = bool(headerFnameRE.match(fname))

  lineNumber: int = 0

  for line in readLines(fname):
    lineNumber += 1

    def complainFL(msg: str) -> None:
      complain(fname, lineNumber, msg)

    if "\r" in line:
      complainFL("Has carriage return (0x0D) character.")

    if isHeader:
      if usingNamespaceRE.match(line):
        complainFL("'using namespace' at top level.")


def main() -> None:
  parser = argparse.ArgumentParser()

  parser.add_argument("--ignore",
    help="Regex of specified file names to ignore.")

  parser.add_argument("files", nargs="+",
    help="Files to check.")

  opts = parser.parse_args()

  # Files to check.
  specifiedFiles = opts.files

  # Possibly ignore some of the specified files.
  if opts.ignore is not None:
    ignoreRE = re.compile(opts.ignore)
    specifiedFiles = (
      [h for h in specifiedFiles if not ignoreRE.search(h)])
  debugPrint(f"specifiedFiles: {specifiedFiles}")

  for fname in specifiedFiles:
    checkFile(fname)

  global numErrors
  if numErrors > 0:
    print(f"Found {numErrors} violations.")
    sys.exit(2)


if __name__ == "__main__":
  call_main(main)


# EOF
