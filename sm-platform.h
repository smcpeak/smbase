// sm-platform.h
// Platform detection.

#ifndef SMBASE_SM_PLATFORM_H
#define SMBASE_SM_PLATFORM_H


// True if the compiler's target platform is Windows.
//
// The type is 'int' for compatibility with C.
#ifdef __WIN32__
#  define PLATFORM_IS_WINDOWS 1
#else
#  define PLATFORM_IS_WINDOWS 0
#endif


// True when the POSIX API is available.
#ifdef __unix__
#  define PLATFORM_IS_POSIX 1
#else
#  define PLATFORM_IS_POSIX 0
#endif


#endif // SMBASE_SM_PLATFORM_H
