#!/usr/bin/env python3
"""
This script accepts a compilation command line and writes a
compile_commands.json fragment to standard output.  The fragment is a
JSON object enclosed by braces and ending with a comma, since it will be
concatenated into a JSON array later.
"""

# This script was written in collaboration with ChatGPT.

import ctypes                          # CDLL
import json                            # dumps
import os                              # getcwd
import subprocess                      # check_output
import sys                             # stderr, argv, exit

from typing import Any, Optional, List


# When true, if we need to transform paths, use the Cygwin DLL already
# loaded into this Cygwin Python process.  Otherwise, call out to the
# `cygpath` program, which is fairly slow.
use_cygwin_dll: bool = True


# Loaded Cygwin DLL if it is loaded.
cygwin: Optional[ctypes.CDLL] = None


def load_cygwin_if_needed() -> None:
  """Load the Cygwin DLL if we have not already.  This is only called
  when we know we are running inside Cygwin Python."""

  global cygwin

  if cygwin is not None:
    return

  # Load the DLL.
  cygwin = ctypes.CDLL("cygwin1.dll")

  # int cygwin_conv_path(int what, const void *from, void *to, size_t size);
  cygwin.cygwin_conv_path.argtypes = [ctypes.c_int,
                                      ctypes.c_void_p,
                                      ctypes.c_void_p,
                                      ctypes.c_size_t]
  cygwin.cygwin_conv_path.restype = ctypes.c_ssize_t

  # int *__errno();
  cygwin.__errno.restype = ctypes.POINTER(ctypes.c_int)

  # char *sterror(int errnum);
  cygwin.strerror.argtypes = [ctypes.c_int]
  cygwin.strerror.restype = ctypes.c_char_p


def cygwin_errno() -> int:
  """Get the value of `errno` as the Cygwin API sees it."""

  assert cygwin is not None
  ptr = cygwin.__errno()
  return ptr.contents.value


def cygwin_strerror(n: int) -> str:
  """Convert a Cygwin errno code into a human-readable message."""

  assert cygwin is not None
  return cygwin.strerror(n).decode('utf-8')


def cygwin_to_mixed_path(src_path: str) -> str:
  """Convert `path` to a Cygwin "mixed" Windows path."""

  if use_cygwin_dll:
    load_cygwin_if_needed()
    assert cygwin is not None

    src_path_bytes = src_path.encode('utf-8')

    bufsize = 4096
    dest_buffer = ctypes.create_string_buffer(bufsize)

    CCP_POSIX_TO_WIN_A = 0               # "ANSI" (8-bit) output.
    CCP_RELATIVE = 0x100                 # Preserve relative paths.
    flags = CCP_POSIX_TO_WIN_A | CCP_RELATIVE

    rc = cygwin.cygwin_conv_path(flags,
                                 ctypes.c_char_p(src_path_bytes),
                                 dest_buffer,
                                 bufsize)
    if rc != 0:
      errno = cygwin_errno()
      msg = cygwin_strerror(errno)
      raise RuntimeError(
        f"`cygwin_conv_path` failed with rc={rc}, errno={errno}, msg={msg!r}")

    dest_path = dest_buffer.value.decode('utf-8')

    # The effect of the `-m` flag to `cygpath` has to be done ourselves
    # since it is not part of `cygwin_conv_path`.
    mixed_path = dest_path.replace("\\", "/")

    return mixed_path

  else:
    # This works, but is fairly slow: making ~400 calls takes ~20s on my
    # machine, or about 0.05s per call.  When using the DLL directly,
    # the same ~400 calls take about 1s total.
    #
    # I keep it here for reference, as a possible fallback if I run into
    # problems with the DLL approach, and for ad-hoc performance
    # comparison.
    b = subprocess.check_output(['cygpath', '-m', src_path])
    return b.decode().strip()


def main(argv: List[str]) -> None:
  if len(argv) < 3:
    print("Usage: make-cc-json.py <compiler> [<args>...] <source-file>",
          file=sys.stderr)
    sys.exit(2)

  cwd: str = os.getcwd()
  arguments: List[str] = argv[1:]
  file: str = argv[-1]

  if sys.platform == "cygwin":
    # When running under Cygwin, `cwd` will be a Cygwin path.  But I'm
    # always using a native `clangd`, which will not accept those paths,
    # so transform it to a Windows path.
    cwd = cygwin_to_mixed_path(cwd)

    # Convert the file path too.
    file = cygwin_to_mixed_path(file)

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
  main(sys.argv)


# EOF
