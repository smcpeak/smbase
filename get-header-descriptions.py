#!/usr/bin/env python3
"""Get header descriptions and put them into index.html."""

import argparse              # argparse
import difflib               # difflib.unified_diff
import html                  # html.escape
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


def readLinesNoNL(filename):
  """Get the lines in 'filename' without newlines."""
  with open(filename) as file:
    return [line.rstrip("\n") for line in file.readlines()]


def writeLines(filename, lines):
  """Write 'lines', each followed by a newline, to 'filename'."""
  with open(filename, "w") as file:
    for line in lines:
      print(line, file=file)


def warn(msg):
  print(f"warning: {msg}", file=sys.stderr)


# Regex to recognize the phrase I use in my copyright comments.
copyrightRE = re.compile(r"copyright and terms of use")

# Regex to match a comment line.
descriptionCommentRE = re.compile(r"// (.*)")


def getHeaderDescriptionHTML(headerFname):
  """Extract the description from 'headerFname' as HTML."""

  # Read the header.
  headerLines = readLinesNoNL(headerFname)

  # Skip the title line.
  lineNo = 1

  # Skip the copyright line if present.
  if copyrightRE.search(headerLines[lineNo]):
    lineNo += 1

  # Description preamble.  The "AUTO" is meant as a reminder that the
  # text is automatically generated, so not to edit it.
  descriptionLinesHTML = [
    f"  <!-- AUTO --><dt><a href=\"{headerFname}\">{headerFname}</a>",
    f"  <!-- AUTO --><dd>",
  ]

  # Get the contiguous comments after that.
  while m := descriptionCommentRE.search(headerLines[lineNo]):
    text = m.group(1)
    textHTML = html.escape(text, quote=False)
    descriptionLinesHTML.append(f"  <!-- AUTO -->  {textHTML}")
    lineNo += 1

  return descriptionLinesHTML


# Match a line that is above a section to insert.
beginHeaderRE = re.compile(r"<!-- begin header: (.*) -->")

# Line that is below an inserted section.
endHeaderRE = re.compile(r"<!-- end header -->")

# As a minor convenience, I insert this for a set of completely new
# headers, and it turns into begin/end pairs.
newSectionsRE = re.compile(r"<!-- new headers: (.*) -->")


def main():
  # Parse command line.
  parser = argparse.ArgumentParser()
  opts = parser.parse_args()

  # Read the document we will modify.
  oldLines = readLinesNoNL("index.html")

  # Modified document.
  newLines = []

  # True if we are scanning to the end of an insertion section.
  scanningForEnd = False

  # Header we are working on.
  headerFname = None

  # Scan all of its lines.  When we find a begin/end, remove what is
  # currently between them and insert the found lines.
  oldLineNo = 0
  for oldLine in oldLines:
    oldLineNo += 1

    # End of the scanning section?
    if scanningForEnd:
      if endHeaderRE.search(oldLine):
        # Reset scan status.
        scanningForEnd = False
        headerFname = None

    # Indicator to insert completely new sections?
    if m := newSectionsRE.search(oldLine):
      headerFnames = m.group(1)

      for headerFname in headerFnames.split():
        newLines.append(f"<!-- begin header: {headerFname} -->")
        newLines += getHeaderDescriptionHTML(headerFname)
        newLines.append(f"<!-- end header -->")
        newLines.append("")

      # Do not copy the new section directive.
      continue

    # Copy lines from old to new.
    if not scanningForEnd:
      newLines.append(oldLine)

    # Start of a scanning section?
    if m := beginHeaderRE.search(oldLine):
      headerFname = m.group(1)

      # Copy the extracted description.
      newLines += getHeaderDescriptionHTML(headerFname)

      scanningForEnd = True

  if scanningForEnd:
    die(f"index.html: Did not find end of '{headerFname}'.")

  # Make a backup of index.html.
  writeLines("index.html.bak", oldLines)

  # Write the new document.
  writeLines("index.html", newLines)


call_main()


# EOF
