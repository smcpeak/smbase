// sm-windows.h
// Wrapper around windows.h.

// When compiling for Windows, this simply #includes windows.h.
//
// When compiling for other platforms, this declares a subset of the
// Windows API sufficient to compile code that uses the Windows API from
// within an "if (PLATFORM_IS_WINDOWS) {...}" conditional block, which
// will be removed by the compiler (even without optimization enabled).

#ifndef SMBASE_SM_WINDOWS_H
#define SMBASE_SM_WINDOWS_H

#include "sm-platform.h"               // PLATFORM_IS_WINDOWS


#if PLATFORM_IS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>                   // Windows API


#else // PLATFORM_IS_WINDOWS

// The declarations here are based on those that appear in the Windows
// API documentation, but are not complete or accurate; they are just
// what I need to compile (and discard) the bits of Windows-calling code
// I have.  It is fine to make ad-hoc additions and changes as needed.

#define STATUS_CONTROL_C_EXIT 0xC000013AU
typedef char const *LPCSTR;
typedef char *LPSTR;
typedef bool BOOL;
typedef unsigned DWORD;
typedef DWORD *LPDWORD;
typedef void *LPVOID;
typedef void *HANDLE;
typedef void *LPSECURITY_ATTRIBUTES;
typedef struct {
  int cb;
} STARTUPINFOA;
typedef STARTUPINFOA *LPSTARTUPINFOA;
typedef struct {
  HANDLE hProcess;
  HANDLE hThread;
} PROCESS_INFORMATION;
typedef PROCESS_INFORMATION *LPPROCESS_INFORMATION;
BOOL CreateProcessA(
  LPCSTR                lpApplicationName,
  LPSTR                 lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCSTR                lpCurrentDirectory,
  LPSTARTUPINFOA        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
);
DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD  dwMilliseconds
);
BOOL GetExitCodeProcess(
  HANDLE  hProcess,
  LPDWORD lpExitCode
);
BOOL CloseHandle(
  HANDLE hObject
);

#endif // PLATFORM_IS_WINDOWS

#endif // SMBASE_SM_WINDOWS_H
