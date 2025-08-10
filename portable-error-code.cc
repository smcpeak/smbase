// portable-error-code.cc
// Code for `portable-error-code` module.

#include "portable-error-code.h"       // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DEFINE_ENUMERATION_TO_STRING_OR, RETURN_ENUMERATION_STRING_OR

#include <iostream>                    // std::ostream


OPEN_NAMESPACE(smbase)


DEFINE_ENUMERATION_TO_STRING_OR(
  PortableErrorCode,
  PortableErrorCode::NUM_REASONS,
  (
    "PEC_NO_ERROR",
    "PEC_FILE_NOT_FOUND",
    "PEC_ACCESS_DENIED",
    "PEC_OUT_OF_MEMORY",
    "PEC_SEGFAULT",
    "PEC_FORMAT",
    "PEC_INVALID_ARGUMENT",
    "PEC_READ_ONLY",
    "PEC_ALREADY_EXISTS",
    "PEC_AGAIN",
    "PEC_BUSY",
    "PEC_INVALID_FILENAME",
    "PEC_UNKNOWN",
  ),
  "<invalid PortableErrorCode>"
)


std::ostream &operator<<(std::ostream &os, PortableErrorCode r)
{
  return os << toString(r);
}


char const *portableCodeDescription(PortableErrorCode r)
{
  RETURN_ENUMERATION_STRING_OR(
    PortableErrorCode,
    PortableErrorCode::NUM_REASONS,
    (
      "No error occurred",
      "File not found",
      "Access denied",
      "Out of memory (maybe)",    // always a suspicious message
      "Invalid pointer address",
      "Invalid data format",
      "Invalid argument",
      "Attempt to modify read-only data",
      "The object already exists",
      "Resource is temporarily unavailable",
      "Resource is busy",
      "File name is invalid (too long, or bad chars, or ...)",
      "Unknown or unrecognized error",
    ),
    r,
    "<bug -- invalid PortableErrorCode>"
  )
}


CLOSE_NAMESPACE(smbase)


// EOF
