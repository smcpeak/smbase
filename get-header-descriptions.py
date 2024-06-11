#!/usr/bin/env python3
"""Get header descriptions and put them into index.html."""

import argparse              # argparse
import difflib               # difflib.unified_diff
import glob                  # glob.glob
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


def readLinesNoNL(filename: str) -> list[str]:
  """Get the lines in 'filename' without newlines."""
  with open(filename) as file:
    return [line.rstrip("\n") for line in file.readlines()]


def writeLines(filename: str, lines: list[str]) -> None:
  """Write 'lines', each followed by a newline, to 'filename'."""
  with open(filename, "w") as file:
    for line in lines:
      print(line, file=file)


def warn(msg: str) -> None:
  print(f"warning: {msg}", file=sys.stderr)


# Regex to recognize the phrase I use in my copyright comments.
copyrightRE = re.compile(r"copyright and terms of use")

# Regex to match a comment line:
#
#   Starts with one of:
#
#     m4_dnl //     An M4 comment followed by a C++ comment.
#     //            C++ comment.
#     /*            Start of C multi-line comment
#      *            Continuation of C multi-line comment
#
#   Then an optional space, then the text.
#
descriptionCommentRE = re.compile(r"(?:m4_dnl //|//|[/ ]\*) ?(.*)")


def getHeaderDescriptionHTML(headerFname: str) -> list[str]:
  """Extract the description from 'headerFname' as HTML lines."""

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
  extractedLines = 0
  while m := descriptionCommentRE.match(headerLines[lineNo]):
    text = m.group(1)
    extractedLines += 1

    # Recognize " */" at the end of a string, which should be stripped.
    if text.endswith(" */"):
      text = text[:-3]

    textHTML = html.escape(text, quote=False)
    if len(textHTML) > 0:
      textHTML = "  " + textHTML
    else:
      # Treat a comment with no text as a paragraph boundary.
      textHTML = "<br><br>"

    # Translate backtick `symbol` into <code>symbol</code>.
    textHTML = re.sub(r"`([^`]+)`", r"<code>\1</code>", textHTML);

    descriptionLinesHTML.append(f"  <!-- AUTO -->{textHTML}")
    lineNo += 1

  if extractedLines == 0:
    die(f"{headerFname}: Did not find any description lines.")

  return descriptionLinesHTML


def checkMentionedHeaders(mentionedHeaders: list[str]) -> None:
  """Check that the set of mentioned headers matches what is in the
  current directory."""

  # Get what is in the current directory.
  curDirHeaders = glob.glob("*.h")

  # Number of issues found.
  numIssues = 0

  # Regex for file names that generally should not be documented
  # independently.
  ignoredRE = re.compile(r"-fwd\.h")

  # Sort both lists.
  mentionedHeaders = sorted(mentionedHeaders)
  curDirHeaders = sorted(curDirHeaders)

  # Avoid having a special case at the end.
  mentionedHeaders.append("sentinel")
  curDirHeaders.append("sentinel")

  # Simultaneously walk both lists.
  mIndex = 0
  cIndex = 0
  while (mIndex < len(mentionedHeaders) and
         cIndex < len(curDirHeaders)):
    mh = mentionedHeaders[mIndex]
    ch = curDirHeaders[cIndex]

    if mh < ch:
      print(f"Mentioned header {mh} is not in current directory.")
      mIndex += 1
      numIssues += 1

    elif mh > ch:
      if not ignoredRE.search(ch):
        print(f"Current directory contains {ch} but index.html does "+
              "not mention it.")
        numIssues += 1
      cIndex += 1

    else:
      mIndex += 1
      cIndex += 1

  if numIssues > 0:
    die(f"There were {numIssues} discrepancies between what was found "+
        "in the current directory and what is mentioned in index.html.")


# Match a line that is above a section to insert.
beginHeaderRE = re.compile(r"<!-- begin header: (.*) -->")

# Line that is below an inserted section.
endHeaderRE = re.compile(r"<!-- end header -->")

# As a minor convenience, I insert this for a set of completely new
# headers, and it turns into begin/end pairs.
newSectionsRE = re.compile(r"<!-- new headers: (.*) -->")

# Line that mentions a header but ignores it.  This is for headers that
# should not be documented independently.
ignoreHeaderRE = re.compile(r"<!-- ignored header: (.*) -->")


def main() -> None:
  # Parse command line.
  parser = argparse.ArgumentParser()
  parser.add_argument("--check", action="store_true",
    help="Check if the descriptions are up to date; do not change anything.")
  opts = parser.parse_args()

  # Read the document we will modify.
  oldLines: list[str] = readLinesNoNL("index.html")

  # Modified document.
  newLines: list[str] = []

  # Set (as a list) of header files we've seen mentioned.
  mentionedHeaders: list[str] = []

  # True if we are scanning to the end of an insertion section.
  scanningForEnd = False

  # Header we are working on.
  headerFname = None

  # Scan all of its lines.  When we find a begin/end, remove what is
  # currently between them and insert the found lines.
  oldLineNo = 0
  for oldLine in oldLines:
    oldLineNo += 1

    try:
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
          mentionedHeaders.append(headerFname)

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
        mentionedHeaders.append(headerFname)

        # Copy the extracted description.
        newLines += getHeaderDescriptionHTML(headerFname)

        scanningForEnd = True

      # Ignore header?
      if m := ignoreHeaderRE.search(oldLine):
        headerFname = m.group(1)
        mentionedHeaders.append(headerFname)

    except BaseException as e:
      die(f"index.html:{oldLineNo}: {exceptionMessage(e)}")

  if scanningForEnd:
    die(f"index.html: Did not find end of '{headerFname}'.")

  checkMentionedHeaders(mentionedHeaders)

  if opts.check:
    if oldLines == newLines:
      print("index.html is up to date.")
      exit(0)
    else:
      print("index.html needs to be regenerated.")
      print(f"Run {sys.argv[0]} to update it.")
      exit(2)

  # Make a backup of index.html.
  writeLines("index.html.bak", oldLines)

  # Write the new document.
  writeLines("index.html", newLines)


call_main()


# EOF
