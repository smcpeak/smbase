#!/usr/bin/env python3
"""
Given a set of .d file created by gcc -MMD, find any cases where a file
depends on a file that is not checked in to the repo.  Emit those as
Makefile dependencies in sorted order.
"""

import argparse              # argparse
import io                    # io.TextIOWrapper
import os                    # os.getenv
import re                    # re.compile
import signal                # signal.signal
import subprocess            # subprocess.Popen
import sys                   # sys.argv, sys.stderr
import traceback             # traceback.print_exc

from typing import cast, TextIO


# -------------- BEGIN: boilerplate -------------
# These are things I add at the start of every Python program to
# allow better error reporting.

# Positive if debug is enabled, with higher values enabling more printing.
debugLevel = 0
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

def die(message: str) -> None:
  """Throw a fatal Error with message."""
  raise Error(message)

def exceptionMessage(e: BaseException) -> str:
  """Turn exception 'e' into a human-readable message."""
  t = type(e).__name__
  s = str(e)
  if s:
    return f"{t}: {s}"
  else:
    return f"{t}"

def call_main() -> None:
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


def read_file_stripped_lines(fname: str) -> list[str]:
  """Read the lines of `fname` into an array, removing leading and
  trailing whitespace from each."""

  with open(fname, "r") as f:
    return [line.strip() for line in f]


def join_backslashed_lines(lines: list[str]) -> list[str]:
  """Join adjacent lines when the first ends in a backslash."""

  result: list[str] = []
  accumulator: str = ""
  continuation: bool = False

  for line in lines:
    if line.endswith("\\"):
      # Add this line to the accumulator, without the backslash.
      accumulator += line[:-1]
      continuation = True

    else:
      if continuation:
        # We had previous lines to join and now finish.
        accumulator += line
        result.append(accumulator)
        accumulator = ""
        continuation = False

      else:
        # This line is not continued and was not preceded by a
        # continuation.
        result.append(line)

  if continuation:
    # The last line had a backslash; emit the accumulator.
    result.append(accumulator)

  return result


assert(join_backslashed_lines(
  ['a', 'b \\', 'c', 'd \\', 'e \\', 'f', 'g']) ==
  ['a', 'b c', 'd e f', 'g']);

assert(join_backslashed_lines(['a \\']) == ['a ']);


# ---- Main ----
# Match the line specifying the target file.  Groups:
#   1: Name of target file.
#   2: Space-separated list of dependencies, which might be empty.
targetRE = re.compile(r"^([^:]+):(.*)$")

# Set of exclusions, as a map to True.
exclusions: dict[str, bool] = {}

# Set of inclusions, as a map to True.  An "inclusion" is a file upon
# which we will emit a dependency even if it is checked in.
inclusions: dict[str, bool] = {}

# List of dependencies discovered, as "TARGET: DEP".
dependencyLines: list[str] = []

def main() -> None:
  # Write only LF line endings to stdout, even on Windows.
  #
  # This is important because the output will be checked into the repo.
  #
  # Previously, I used a method based on the answer at:
  #
  #   https://stackoverflow.com/questions/34960955/print-lf-with-python-3-to-windows-stdout
  #
  # but more recent (?) mypy does not like it.  ChatGPT suggested the
  # code used now, although I'm not easily able to properly test it.
  sys.stdout = io.TextIOWrapper(
    sys.stdout.buffer,
    encoding='utf-8',
    newline='\n')

  # Parse command line.
  parser = argparse.ArgumentParser()
  parser.add_argument("--exclude", action="append", metavar="DEP",
    help="Exclude a dependency.  Can be repeated.")
  parser.add_argument("--include", action="append", metavar="DEP",
    help="Include as a dependency even when checked in.  Can be repeated.")
  parser.add_argument("--add", action="append", metavar="RULE",
    help="Add RULE like 'target: depfile' even if .d files do not mention it. "+
         "Meant for dependencies that are conditional upon build configuration. "+
         "Can be repeated.")
  parser.add_argument("files", nargs="+", metavar="file.d",
    help="Dependency files to analyze, created using 'gcc -MMD'.")
  opts = parser.parse_args()

  # Process inclusions and exclusions.
  if opts.include:
    for x in opts.include:
      inclusions[x] = True
  if opts.exclude:
    for x in opts.exclude:
      exclusions[x] = True

  # Gather set of files in the repository.
  getRepoFiles()

  # Process the .d files.
  for file in opts.files:
    debugPrint(f"processing .d file: {file}")

    # Get logical lines.
    lines: list[str] = join_backslashed_lines(read_file_stripped_lines(file))

    for line in lines:
      m = targetRE.match(line)
      if m:
        (target, deps) = m.group(1,2)

        for element in deps.split():
          if element.startswith("../"):
            # File in another directory.
            pass
          elif fileInRepo(element) and element not in inclusions:
            # File checked in, and not explicitly included; no need to
            # emit dependency.
            pass
          elif element in exclusions:
            # Explicitly excluded.
            pass
          elif sameWithoutExtensions(target, element):
            # The target probably already depends on the element due to
            # another rule like the ordinary compilation rule.
            pass
          else:
            dependencyLines.append(f"{target}: {element}")

      else:
        die(f"Unexpected line: {line}")


  # Add the --add rules.
  if opts.add:
    for addition in opts.add:
      if addition not in dependencyLines:
        dependencyLines.append(addition)

  # This sorts in Unicode code point order, so should be stable across
  # environments.  The reason to sort is I intend to check the output
  # in to the repository, and I want subsequent updates to look nice
  # when viewed as a diff.
  dependencyLines.sort()

  for dep in dependencyLines:
    print(f"{dep}")


# ---- Repo Queries ----
# Map from files in the repo to True.
repoFiles: dict[str, bool] = {}

def getRepoFiles() -> None:
  """Call 'git ls-files' and add the output to 'reopFiles'."""
  command = ["git", "ls-files", "--recurse-submodules"]

  try:
    lines = subprocess.check_output(command).decode()
    for file in lines.split("\n"):
      if file != "":
        debugPrint(f"repo file: {file}")
        repoFiles[file] = True

  except BaseException as e:
    commandString = " ".join(command)
    die(f"While running \"{commandString}\": {exceptionMessage(e)}")

def fileInRepo(file: str) -> bool:
  """True if 'file' is in the repository."""
  return file in repoFiles


# ---- String Utilities ----
# Match a file name and its extension.  Groups:
#   1: Everything before the final ".".
#   2: Everything after the final ".".
fileExtensionRE = re.compile(r"^(.*)\.([^.]*)$")

def stripExtension(name: str) -> str:
  """Return 'name' without its file name extension."""
  m = fileExtensionRE.match(name)
  if m:
    return m.group(1)
  else:
    # No extension.
    return name

def sameWithoutExtensions(f1: str, f2: str) -> bool:
  """True if 'f1' and 'f2' are the same after removing extensions."""
  return stripExtension(f1) == stripExtension(f2)


call_main()


# EOF
