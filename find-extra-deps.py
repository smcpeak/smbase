#!/usr/bin/env python3
"""
Given a set of .d file created by gcc -MMD, find any cases where a file
depends on a file that is not checked in to the repo.  Emit those as
Makefile dependencies in sorted order.
"""

import argparse              # argparse
import os                    # os.getenv
import re                    # re.compile
import signal                # signal.signal
import subprocess            # subprocess.Popen
import sys                   # sys.argv, sys.stderr
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


# ---- Main ----
# Match the line specifying the target file.  Groups:
#   1: Name of target file.
#   2: Space-separated list of first set of dependencies, possibly
#      including a continuation backslash.
targetRE = re.compile(r"^([^:]+):\s+(.*)$")

# Set of exclusions, as a map to True.
exclusions = {}

# Set of inclusions, as a map to True.  An "inclusion" is a file upon
# which we will emit a dependency even if it is checked in.
inclusions = {}

# List of dependencies discovered, as "TARGET: DEP".
dependencyLines = []

def main():
  # Write only LF line endings to stdout, even on Windows.
  # https://stackoverflow.com/questions/34960955/print-lf-with-python-3-to-windows-stdout
  #
  # This is important because the output will be checked into the repo.
  sys.stdout = open(sys.__stdout__.fileno(),
                    mode=sys.__stdout__.mode,
                    buffering=1,
                    encoding=sys.__stdout__.encoding,
                    errors=sys.__stdout__.errors,
                    newline='\n',
                    closefd=False)

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
    target = None
    deps = []

    def processDeps(d):
      """Split 'd' and add elements to 'deps'."""
      for element in d.split():
        if element == "\\":
          # Continuation backslash.
          pass
        elif element.startswith("../"):
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
          deps.append(element)

    # Process lines in 'file'.
    with open(file) as f:
      for line in f:
        line = line.rstrip("\n")
        debugPrint(f"line: {line}")

        m = targetRE.match(line)
        if m:
          (t, d) = m.group(1,2)
          if target != None:
            die(f"{file}: There is more than one target.")
          target = t

          processDeps(d)

        else:
          if target == None:
            die(f"{file}: Missed target name.")

          # Second or later line after a backslash continuation.
          processDeps(line)

    for dep in deps:
      dependencyLines.append(f"{target}: {dep}")

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
repoFiles = {}

def getRepoFiles():
  """Call 'git ls-files' and add the output to 'reopFiles'."""
  command = ["git", "ls-files"]

  try:
    lines = subprocess.check_output(command).decode()
    for file in lines.split("\n"):
      if file != "":
        debugPrint(f"repo file: {file}")
        repoFiles[file] = True

  except BaseException as e:
    commandString = " ".join(command)
    die(f"While running \"{commandString}\": {exceptionMessage(e)}")

def fileInRepo(file):
  """True if 'file' is in the repository."""
  return file in repoFiles


# ---- String Utilities ----
# Match a file name and its extension.  Groups:
#   1: Everything before the final ".".
#   2: Everything after the final ".".
fileExtensionRE = re.compile(r"^(.*)\.([^.]*)$")

def stripExtension(name):
  """Return 'name' without its file name extension."""
  m = fileExtensionRE.match(name)
  if m:
    return m.group(1)
  else:
    # No extension.
    return name

def sameWithoutExtensions(f1, f2):
  """True if 'f1' and 'f2' are the same after removing extensions."""
  return stripExtension(f1) == stripExtension(f2)


call_main()


# EOF
