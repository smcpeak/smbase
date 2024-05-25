// mypopen-test.c
// Tests for mypopen.

#include "mypopen.h"         // module unde test

#include "dummy-printf.h"    // dummy_printf

#include <stdlib.h>          // exit, perror
#include <stdio.h>           // printf
#include <string.h>          // memcmp

// POSIX
#include <unistd.h>          // read, write


// Silence test.
#define printf dummy_printf


static void die(char const *fn)
{
  perror(fn);
  exit(2);
}


// Called from unit-tests.cc.
void test_mypopen()
{
  char buf[80];
  int stat;

  if (!mypopenModuleWorks()) {
    printf("mypopen module does not work on this platform, skipping test\n");
    return;
  }

  // try cat
  {
    int in, out;
    char const *argv[] = { "cat", NULL };
    int pid = popen_execvp(&in, &out, NULL, argv[0], argv);
    printf("child pid is %d\n", pid);

    if (write(in, "foo\n", 4) != 4) {
      die("write");
    }
    if (read(out, buf, 4) != 4) {
      die("read");
    }

    if (0==memcmp(buf, "foo\n", 4)) {
      printf("cat worked for foo\n");
    }
    else {
      printf("cat FAILED\n");
      exit(2);
    }

    if (write(in, "bar\n", 4) != 4) {
      die("write");
    }
    if (read(out, buf, 4) != 4) {
      die("read");
    }

    if (0==memcmp(buf, "bar\n", 4)) {
      printf("cat worked for bar\n");
    }
    else {
      printf("cat FAILED\n");
      exit(2);
    }

    close(in);
    close(out);

    printf("waiting for cat to exit..\n");
    if (mypopenWait(&stat) < 1) {
      perror("wait");
    }
    else {
      printf("cat exited with status %d\n", stat);
    }
  }

  // try something which fails
  {
    int in, out, err;
    int len;
    char const *argv[] = { "does_not_exist", NULL };
    int pid = popen_execvp(&in, &out, &err, argv[0], argv);
    printf("child pid is %d\n", pid);

    printf("waiting for error message...\n");
    len = read(err, buf, 78);
    if (len < 0) {
      die("read");
    }
    if (buf[len-1] != '\n') {
      buf[len++] = '\n';
    }
    buf[len] = 0;
    printf("error string: %s", buf);   // should include newline from perror

    close(in);
    close(out);
    close(err);

    printf("waiting for child to exit..\n");
    if (mypopenWait(&stat) < 1) {
      perror("wait");
    }
    else {
      printf("child exited with status %d\n", stat);
    }
  }

  // also fails, but with stdout and stderr going to same pipe
  {
    int in, out;
    int len;
    char const *argv[] = { "does_not_exist", NULL };
    int pid = popen_execvp(&in, &out, &out, argv[0], argv);
    printf("out==err: child pid is %d\n", pid);

    printf("waiting for error message...\n");
    len = read(out, buf, 78);
    if (len < 0) {
      die("read");
    }
    if (buf[len-1] != '\n') {
      buf[len++] = '\n';
    }
    buf[len] = 0;
    printf("error string: %s", buf);   // should include newline from perror

    close(in);
    close(out);

    printf("waiting for child to exit..\n");
    if (mypopenWait(&stat) < 1) {
      perror("wait");
    }
    else {
      printf("child exited with status %d\n", stat);
    }
  }

  printf("mypopen worked!\n");
}


// EOF
