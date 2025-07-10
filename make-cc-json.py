#!/usr/bin/env python3
"""
This script accepts a compilation command line and writes a
compile_commands.json fragment to standard output.  The fragment is a
JSON object enclosed by braces and ending with a comma, since it will be
concatenated into a JSON array later.
"""

# Most of this script was written by ChatGPT, using something close to
# the above description as the prompt (plus an input+output example).  I
# then lightly modified the output.

import sys
import os
import json
from typing import List

def main(argv: List[str]) -> None:
  if not argv:
    print("Usage: make-cc-json.py <compiler> [<args>...] <source-file>", file=sys.stderr)
    sys.exit(2)

  cwd: str = os.getcwd()
  arguments: List[str] = argv
  file: str = argv[-1]

  fragment = {
    "directory": cwd,
    "arguments": arguments,
    "file": file
  }

  # Output as five lines.
  print("{")
  print(f'  "directory": {json.dumps(fragment["directory"])},')
  print(f'  "file": {json.dumps(fragment["file"])}')
  print(f'  "arguments": {json.dumps(fragment["arguments"])},')
  print("},")

if __name__ == "__main__":
  main(sys.argv[1:])


# EOF
