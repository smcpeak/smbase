// portable-error-code.cc
// Code for `portable-error-code` module.

#include "portable-error-code.h"       // this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, DEFINE_ENUMERATION_TO_STRING_OR, RETURN_ENUMERATION_STRING_OR

#include <iostream>                    // std::ostream


OPEN_NAMESPACE(smbase)


DEFINE_ENUMERATION_TO_STRING_OR(
  SysErrorReasonCode,
  SysErrorReasonCode::NUM_REASONS,
  (
    "R_NO_ERROR",
    "R_FILE_NOT_FOUND",
    "R_ACCESS_DENIED",
    "R_OUT_OF_MEMORY",
    "R_SEGFAULT",
    "R_FORMAT",
    "R_INVALID_ARGUMENT",
    "R_READ_ONLY",
    "R_ALREADY_EXISTS",
    "R_AGAIN",
    "R_BUSY",
    "R_INVALID_FILENAME",
    "R_UNKNOWN",
  ),
  "<invalid SysErrorReasonCode>"
)


std::ostream &operator<<(std::ostream &os, SysErrorReasonCode r)
{
  return os << toString(r);
}


char const *reasonCodeDescription(SysErrorReasonCode r)
{
  RETURN_ENUMERATION_STRING_OR(
    SysErrorReasonCode,
    SysErrorReasonCode::NUM_REASONS,
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
    "<bug -- invalid SysErrorReasonCode>"
  )
}


CLOSE_NAMESPACE(smbase)


// EOF
