// sm-unixutil.h            see license.txt for copyright and terms of use
// some utilities on top of unix functions

// 2024-05-19: This module was previously called 'unixutil'.  I renamed
// it to 'sm-unixutil' in part to avoid potential name clashes with
// other software, and in part because on Windows its existence
// interfered with file name completion of "./unit-tests" since the .o
// and .d files end up having the executable bit set due to Cygwin
// stuff.

#ifndef SMBASE_SM_UNIXUTIL_H
#define SMBASE_SM_UNIXUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

// write entire contents of buffer to 'fd', returning 0 on failure
int writeAll(int fd, void const *buf, int len);

// read(2) some data into a buffer of 'len' bytes; null-terminate
// those bytes, and strip any trailing newline; return 0 on failure
int readString(int fd, char *str, int len);

// test if there are bytes available to be read from a file descriptor,
// returning nonzero if so
int canRead(int fd);


#ifdef __cplusplus
}
#endif

#endif // SMBASE_SM_UNIXUTIL_H
