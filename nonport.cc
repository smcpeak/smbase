// nonport.cc            see license.txt for copyright and terms of use
// code for nonport.h
// Scott McPeak and Dan Bonachea, 1998-1999  This file is public domain.

#include <stdio.h>       // printf
#include <stdlib.h>      // abort, exit
#include <string.h>      // strncpy

#ifdef __WIN32__
#  ifdef USE_MINWIN_H
#    include "minwin.h"   // api
#  else
#    include <windows.h>  // api
#  endif

#  include <conio.h>      // getch or _getch
#  include <dos.h>        // sleep
#  include <io.h>         // chmod
#  ifdef __BORLANDC__
#    include <dir.h>      // mkdir, chdir
#    pragma warn -par     // warning: parameter not used
#  else    // MSVC
#    include <errno.h> // ENOENT
#    include <direct.h>   // _mkdir, _chdir
#    define getch _getch
#    define mkdir _mkdir    // why must VC be such an oddball?
#    define chdir _chdir
#    define chmod _chmod
     static void sleep(unsigned sec) {
       Sleep(sec * 1000);
       return;
     }
#  endif
#  define DIRSLASH '\\'
#  define DIRSLASHES "\\/"

#else   // unix
#  include <sys/time.h>   // struct timeval, gettimeofday
#  include <sys/types.h>  // mkdir constants, DIR, struct dirent
#  include <fcntl.h>      // mkdir constants
#  include <unistd.h>     // mkdir, sleep, chdir, geteuid
#  include <errno.h>      // errno
#  include <pwd.h>        // getpwuid, struct passwd
#  define DIRSLASH '/'
#  define DIRSLASHES "/"

#endif

#include <sys/stat.h>     // chmod, mode macros
#include <time.h>         // tzset, localtime, time
#include "sm-iostream.h"  // cout
#include "sm-macros.h"    // PRETEND_USED

#if !defined(__WIN32__) || defined(__BORLANDC__)
  #include <dirent.h>       // opendir
#endif

#include "nonport.h"      // this module


NonportFailFunc nonportFail = defaultNonportFail;

void defaultNonportFail(char const *, char const *)
{}

// convenience
inline void fail(char const *call, char const *ctx=NULL)
{
  nonportFail(call, ctx);
}


void setRawMode(bool raw)
{
# ifdef __WIN32__
    // nothing necessary; getConsoleChar handles it
    PRETEND_USED(raw);

# else
    int res;
    if (raw) {
      // turn off UNIX term output, put in raw mode
      res = system("stty -echo raw");
    }
    else {
      // back to normal mode
      res = system("stty echo -raw");
    }

    if (res != 0) {
      //fail("system", "setRawMode");
    }
# endif
}


// get the next character typed, without any intervening interpretation
// or buffering
char getConsoleChar()
{
# ifdef __WIN32__
    // this function always bypasses 'cooked' console mode
    return (char)getch();

# else
    // relies on raw mode for console
    int ch = getchar();
    if (ch == EOF) {
      fail("getchar", "getConsoleChar");
    }
    return ch;
# endif
}


// return the # of milliseconds since some unspecified, but
// constant for the life of the program, event
long getMilliseconds()
{
# ifdef __WIN32__
    // # of milliseconds since program started
    return GetTickCount();

# else
    // some unspecified millisecond count (unspecified
    // because tv.tv_sec * 1000 will overflow a 32-bit
    // long, for typical values)
    struct timeval tv;
    gettimeofday(&tv, NULL);
      // according to HPUX man page, this always returns 0 and does
      // not have any errors defined

    //printf("tv.tv_sec = %d, tv.tv_usec = %d\n",
    //       tv.tv_sec, tv.tv_usec);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
# endif
}


bool limitFileAccess(char const *fname)
{
  // read/write for user, nothing for group or others
  if (chmod(fname, 0600) != 0) {
    fail("chmod", fname);
    return false;
  }
  else {
    return true;
  }
}


bool createDirectory(char const *dirname)
{
  int res;
# ifdef __WIN32__
    // no 'mode' argument
    res = mkdir(dirname);
# else   // unix
    // read/write/execute permission for user, no one else
    res = mkdir(dirname, S_IRUSR | S_IWUSR | S_IXUSR);
# endif

  if (res != 0) {
    fail("mkdir", dirname);
    return false;
  }
  else {
    return true;
  }
}


bool changeDirectory(char const *dirname)
{
  if (0!=chdir(dirname)) {
    fail("chdir", dirname);
    return false;
  }
  else {
    return true;
  }
}


bool getCurrentDirectory(char *dirname, int len)
{
  bool ok = getcwd(dirname, len) != NULL;
  if (!ok) {
    fail("getcwd");
  }
  return ok;
}


bool removeFile(char const *fname)
{
  bool ok = unlink(fname) == 0;
  if (!ok) {
    fail("unlink", fname);
  }
  return ok;
}


// this may in fact use a portable subset of the standard
// C library.. but I'm not holding my breath, so that's why
// this routine is in the 'nonport' module
void getCurrentDate(int &month, int &day, int &year)
{
  // tzset is apparently required (recommended?) before
  // calling localtime()
  #if !defined(__CYGWIN__)
  tzset();
  #endif

  // retrieve standard UNIX time
  time_t unixTime;
  time(&unixTime);

  // convert to month/day/year
  struct tm *t = localtime(&unixTime);

  // write into return variables
  month = t->tm_mon + 1;
  day = t->tm_mday;
  year = t->tm_year + 1900;    // this is not a y2k bug!
}


void portableSleep(unsigned seconds)
{
  // with proper headers included (above), "sleep" works
  sleep(seconds);
}


void sleepForMilliseconds(unsigned ms)
{
  #if defined(__WIN32__)
    Sleep(ms);
  #else
    // Let's hope that 'usleep' is available.
    usleep(ms * (useconds_t)1000);
  #endif

  // This depends on C++11, and requires linking with pthreads on
  // Windows (which is implicit with Mingw GCC but not Clang), so I am
  // not using it anymore.
  #if 0
    // Would also be needed at the top:
    #include <chrono>         // std::chrono::milliseconds
    #include <thread>         // std::this_thread

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  #endif
}


void getCurrentUsername(char *buf, int buflen)
{
  #ifdef __WIN32__
    DWORD len = buflen;
    if (!GetUserName(buf, &len)) {
      fail("GetUserName");
      strncpy(buf, "(unknown)", buflen);
    }

  #else    // unix (SunOS only?  we'll see..)
    #if 0     // old.. linux man page insists 'cuserid' is a bad fn to call
      char temp[L_cuserid];
      cuserid(temp);         // fail return?
    #endif // 0

    char const *temp;
    struct passwd *pw = getpwuid(geteuid());
    if (!pw) {
      fail("getpwuid(geteuid())");
      temp = "(unknown)";
    }
    else {
      temp = pw->pw_name;
    }

    strncpy(buf, temp, buflen);
  #endif
}


// loop reading characters, return when finished
static void nonechoLoop(char *buf, int len)
{
  int cursor = 0;
  for(;;) {
    char ch = getConsoleChar();
    switch (ch) {
      case '\r':    // carriage return
        buf[cursor] = 0;
        return;

      case '\b':    // backspace
        if (cursor > 0) {
          cursor--;
        }
        break;

      default:
        buf[cursor++] = ch;
        if (cursor >= len-1) {
          // end of buffer
          buf[len-1] = 0;
          return;
        }
        break;
    }
  }
}


void readNonechoString(char *buf, int len, char const *prompt)
{
  cout << prompt;
  cout.flush();

  setRawMode(true);

  try {
    nonechoLoop(buf, len);
  }
  catch (...) {
    setRawMode(false);    // make sure it gets reset
    throw;
  }

  setRawMode(false);

  cout << "\n";
  cout.flush();
}


void applyToCwdContents(PerFileFunc func, void *extra)
{
  applyToDirContents(".", func, extra);
}


void applyToDirContents(char const *dirName,
                        PerFileFunc func, void *extra)
{
  // SM: we had find{first,next} code here for win32, but
  //     my Borland has opendir & friends, so let's see if
  //     that works everywhere.. (old code is below, in
  //     trash section)
  // DOB: VC doesn't have opendir-
  //  I think this is the only way to do it in the Win32 API
  #if defined(__WIN32__) && !defined(__BORLANDC__)
    struct _finddata_t fb;
    char* buf = new char[strlen(dirName)+5];
    strcpy(buf, dirName);
    if (buf[strlen(buf)-1] != '\\') strcat(buf, "\\");
    strcat(buf, "*");
    intptr_t handle = _findfirst(buf, &fb);
    delete[] buf;
    int done = (handle == -1);
    if (handle == -1 && errno != ENOENT) // ENOENT = no matching entries
      fail("_findfirst", dirName);
    while (!done) {
      if (!func(fb.name, extra)) {
        break;
      }
      done = _findnext(handle, &fb);
    }
    if (handle != -1) {
      if (_findclose(handle)) fail("_findclose", dirName);
    }

  #else     // unix and borland
    DIR *dir = opendir(dirName);
    if (!dir) {
      fail("opendir", dirName);
      return;
    }

    for(;;) {
      // grab next directory entry
      struct dirent *ent = readdir(dir);
      if (!ent) {
        break;     // end of listing
      }

      if (!func(ent->d_name, extra)) {
        break;     // user wants to stop listing
      }
    }

    if (closedir(dir) != 0) {
      fail("closedir", dirName);
    }
  #endif
}


bool isDirectory(char const *path)
{
  struct stat st;
  if (0!=stat(path, &st)) {
    fail("stat", path);
    return false;
  }
  #if defined(__WIN32__) && !defined(__BORLANDC__)
    return !!(st.st_mode & _S_IFDIR); // this is how it works for VC
  #else
    return S_ISDIR(st.st_mode);
  #endif
}


bool getFileModificationTime(char const *path, int64_t &modUnixTime)
{
  struct stat st;
  if (0!=stat(path, &st)) {
    modUnixTime = 0;
    fail("stat", path);
    return false;
  }

  // Assume this is unix time on all systems.
  modUnixTime = (int64_t)st.st_mtime;

  return true;
}


bool fileOrDirectoryExists(char const *path)
{
  struct stat st;
  if (0!=stat(path, &st)) {
    return false;     // assume error is because of nonexistence
  }
  else {
    return true;
  }
}


// adapted from Dan's keyutils.cpp (and indentation style
// ruthlessly changed! :) )
bool ensurePath(char const *filename, bool isDirectory)
{
  // make a temporary buffer so we can modify it safely
  int len = strlen(filename);
  char *temp = new char[len+1];
  strcpy(temp, filename);

  if (isDirectory) {
    len++;    // also consider final '\0' (strchr returns ptr to it)
  }

  // start at 1 (and not 0) because if the path starts with a slash,
  // then starting at 0 will cause us to try mkdir("")
  for (int i=1; i < len; i++) {
    if (strchr(DIRSLASHES, temp[i])) {
      // wherever there is a slash (or '\0'), truncate and test
      temp[i] = '\0';
      if (!fileOrDirectoryExists(temp)) {
        // make directory if necessary; automatically limits file access
        if (!createDirectory(temp)) {
          delete[] temp;
          return false;
        }
      }
      temp[i] = DIRSLASH;      // may kill final '\0', doesn't matter
    }
  }

  // was leaking this.. found the leak with the feudal-C instrumentor!
  delete[] temp;
  return true;
}


// underlying test
bool hsrcHelper()
{
  #if !defined(__WIN32__)     // unix
    // see if /dev/random exists and is readable
    int fd = open("/dev/random", O_RDONLY);
    if (fd < 0) {
      return false;
    }

    // looks ok!
    if (close(fd) < 0) {
      perror("close");
      return false;      // seems unlikely, but..
    }

    return true;

  #else    // windows
    return false;
  #endif
}

bool hasSystemCryptoRandom()
{
  static bool cached = false;
  static bool cachedAnswer;

  if (!cached) {
    cachedAnswer = hsrcHelper();
    cached = true;
  }

  return cachedAnswer;
}


// assume we are only called if the above fn returned true;
// for this fn, any failure is considered fatal because we
// don't have a way to communicate it, and the possibility of
// returning nonrandom values is not tolerable (if something
// I haven't thought of is causing failure, add it to the list
// of things hasSystemCryptoRandom checks)
unsigned getSystemCryptoRandom()
{
  #if !defined(__WIN32__)     // unix
    // open /dev/random
    int fd = open("/dev/random", O_RDONLY);
    if (fd < 0) {
      perror("open");
      exit(2);
    }

    // grab 4 bytes
    union {
      unsigned ret;
      char c[4];
    };
    int got = 0;

    while (got < 4) {
      int ct = read(fd, c+got, 4-got);
      if (ct < 0) {
        perror("read");
        exit(2);
      }
      if (ct == 0) {
        fprintf(stderr, "got 0 bytes from /dev/random.. it's supposed to block!!\n");
        exit(2);
      }
      got += ct;
    }

    if (close(fd) < 0) {
      perror("close");
      exit(2);
    }

    return ret;

  #else     // windows
    fprintf(stderr, "no system crypto random function available!\n");
    exit(2);
    return 0;    // silence warning
  #endif
}


int getProcessId()
{
  #ifdef __WIN32__
    return GetCurrentProcessId();

  #else  // unix
    return getpid();

  #endif
}


// do we have access to C99 vsnprintf?
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
  #if __GLIBC__ >= 3 || (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1)
    // glibc-2.1 or later: yes (glibc-2.0.6 has a function called
    // vsnprintf, but the interface is wrong)
    #define HAS_C99_VSNPRINTF
  #endif
#endif

// note to reader: if your system has C99 VSNPRINTF, feel free to add
// appropriate detection code

#ifndef HAS_C99_VSNPRINTF
  // no vsnprintf, will use gprintf (which is slow, and overestimates sometimes)
  #include "gprintf.h"        // general_vprintf

  static int counting_output_function(void *extra, int /*ch*/)
  {
    // 'extra' is a pointer to the count
    int *ct = (int*)extra;
    (*ct)++;
    return 0;    // no failure
  }
#endif // !HAS_C99_VSNPRINTF

int vnprintf(char const *format, va_list args)
{
  #ifdef HAS_C99_VSNPRINTF
    // can use this directly
    return vsnprintf(NULL, 0, format, args);

  #else
    // conservatively interpret the format string using gprintf
    int count = 0;
    general_vprintf(counting_output_function, &count, format, args);
    return count;
  #endif
}


int nprintf(char const *format, ...)
{
  va_list args;
  va_start(args, format);
  int ret = vnprintf(format, args);
  va_end(args);
  return ret;
}


// EOF
