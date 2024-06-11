#!/usr/bin/env python3
"""Get file descriptions and put them into index.html."""

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


def getFileDescriptionHTML(fileFname: str) -> list[str]:
  """Extract the description from 'fileFname' as HTML lines."""

  # Read the file.
  fileLines = readLinesNoNL(fileFname)

  # Skip the title line.
  lineNo = 1

  # Skip the copyright line if present.
  if copyrightRE.search(fileLines[lineNo]):
    lineNo += 1

  # Description preamble.  The "AUTO" is meant as a reminder that the
  # text is automatically generated, so not to edit it.
  descriptionLinesHTML = [
    f"  <!-- AUTO --><dt><a href=\"{fileFname}\">{fileFname}</a>",
    f"  <!-- AUTO --><dd>",
  ]

  # Get the contiguous comments after that.
  extractedLines = 0
  while m := descriptionCommentRE.match(fileLines[lineNo]):
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
    die(f"{fileFname}: Did not find any description lines.")

  return descriptionLinesHTML


def getFileDescriptionSectionHTML(fname: str) -> list[str]:
  """Return lines of HTML that describe `fname`, including the
  "begin file desc" and "end file desc" section boundaries."""

  ret: list[str] = []

  ret.append(f"<!-- begin file desc: {fname} -->")
  ret += getFileDescriptionHTML(fname)
  ret.append(f"<!-- end file desc -->")
  ret.append("")

  return ret


def checkMentionedFiles(mentionedFiles: list[str],
                        specifiedFiles: list[str]) -> None:
  """Check that the set of `mentionedFiles` matches what is in
  `specifiedFiles`."""

  # Number of issues found.
  numIssues = 0

  # Sort both lists.
  mentionedFiles = sorted(mentionedFiles)
  specifiedFiles = sorted(specifiedFiles)

  # Avoid having a special case at the end.
  mentionedFiles.append("sentinel")
  specifiedFiles.append("sentinel")

  # Simultaneously walk both lists.
  mIndex = 0
  sIndex = 0
  while (mIndex < len(mentionedFiles) and
         sIndex < len(specifiedFiles)):
    mh = mentionedFiles[mIndex]
    sh = specifiedFiles[sIndex]

    if mh < sh:
      print(f"Mentioned file {mh} is not in current directory.")
      mIndex += 1
      numIssues += 1

    elif mh > sh:
      print(f"Specified file {sh} is not mentioned in index.html")
      numIssues += 1
      sIndex += 1

    else:
      mIndex += 1
      sIndex += 1

  if numIssues > 0:
    die(f"There were {numIssues} discrepancies between what was "+
        "specified on the command line and what is mentioned in "+
        "index.html.")


# Match a line that is above a section to insert.
beginFileRE = re.compile(r"<!-- begin file desc: (.*) -->")

# Line that is below an inserted section.
endFileRE = re.compile(r"<!-- end file desc -->")

# As a minor convenience, I insert this for a set of completely new
# files, and it turns into begin/end pairs.
newSectionsRE = re.compile(r"<!-- new file descs: (.*) -->")

# Line that mentions a file but ignores it.  This is for files that
# should not be documented independently.
ignoreFileRE = re.compile(r"<!-- ignored file desc: (.*) -->")

# This goes near the end of the file and marks where to automatically
# insert descriptions of any files specified but not mentioned.
autoAddFilesRE = re.compile(r"<!-- auto-add file descs -->")


def main() -> None:
  # Parse command line.
  parser = argparse.ArgumentParser()

  parser.add_argument("--check", action="store_true",
    help="Check if the descriptions are up to date; do not change anything.")

  # `argparse` is stupid when it comes to option arguments that start
  # with a hyphen:
  #
  #   https://stackoverflow.com/questions/16174992/cant-get-argparse-to-read-quoted-string-with-dashes-in-it
  #
  # So with the given example, it is necessary to use `=` between the
  # option and its argument.
  #
  parser.add_argument("--ignore",
    help="Regex of specified file names to ignore.  Ex: '--ignore=-fwd\\.h$'")

  parser.add_argument("files", nargs="+",
    help="Files that should be mentioned in index.html.")

  opts = parser.parse_args()

  # Files that should be in index.html.
  specifiedFiles = opts.files

  # Possibly ignore some of the specified files.
  if opts.ignore is not None:
    ignoreRE = re.compile(opts.ignore)
    specifiedFiles = (
      [h for h in specifiedFiles if not ignoreRE.search(h)])
    debugPrint(f"specifiedFiles: {specifiedFiles}")

  # Read the document we will modify.
  oldLines: list[str] = readLinesNoNL("index.html")

  # Modified document.
  newLines: list[str] = []

  # Set (as a list) of files we've seen mentioned.
  mentionedFiles: list[str] = []

  # True if we are scanning to the end of an insertion section.
  scanningForEnd = False

  # File we are working on.
  fileFname = None

  # Scan all of its lines.  When we find a begin/end, remove what is
  # currently between them and insert the found lines.
  oldLineNo = 0
  for oldLine in oldLines:
    oldLineNo += 1

    try:
      # End of the scanning section?
      if scanningForEnd:
        if endFileRE.search(oldLine):
          # Reset scan status.
          scanningForEnd = False
          fileFname = None

      # Indicator to insert completely new sections?
      if m := newSectionsRE.search(oldLine):
        fileFnames = m.group(1)

        for fileFname in fileFnames.split():
          mentionedFiles.append(fileFname)
          newLines += getFileDescriptionSectionHTML(fileFname)

        # Do not copy the new section directive.
        continue

      # Indicator to insert anything not mentioned?
      if m := autoAddFilesRE.search(oldLine):
        mentionedFilesSet = dict.fromkeys(mentionedFiles, True)
        for fname in specifiedFiles:
          if fname not in mentionedFilesSet:
            # Do not complain about this file at the end even though it
            # was not actually explicitly mentioned.
            mentionedFiles.append(fname)

            newLines += getFileDescriptionSectionHTML(fname)

        # Here, we *do* continue so that the directive will be copied to
        # the output and hence used the next time we have extra files.

      # Copy lines from old to new.
      if not scanningForEnd:
        newLines.append(oldLine)

      # Start of a scanning section?
      if m := beginFileRE.search(oldLine):
        fileFname = m.group(1)
        mentionedFiles.append(fileFname)

        # Copy the extracted description.
        newLines += getFileDescriptionHTML(fileFname)

        scanningForEnd = True

      # Ignore file?
      if m := ignoreFileRE.search(oldLine):
        fileFname = m.group(1)
        mentionedFiles.append(fileFname)

    except BaseException as e:
      die(f"index.html:{oldLineNo}: {exceptionMessage(e)}")

  if scanningForEnd:
    die(f"index.html: Did not find end of '{fileFname}'.")

  # Compare what we found to what was specified.
  checkMentionedFiles(mentionedFiles, specifiedFiles)

  if oldLines == newLines:
    print("index.html is up to date.")
    exit(0)

  if opts.check:
    print("index.html needs to be regenerated.")
    print(f"Run {sys.argv[0]} to update it.")
    exit(2)

  # Make a backup of index.html.
  writeLines("index.html.bak", oldLines)

  # Write the new document.
  writeLines("index.html", newLines)

  print("Updated index.html.")


call_main()


# EOF
