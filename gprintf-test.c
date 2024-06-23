/* gprintf-test.c */
/* Tests for gprintf. */

#include "gprintf.h"                   /* module under test */

#include <stdio.h>                     /* fputc, printf, vsprintf */
#include <string.h>                    /* strcmp, strlen */
#include <stdlib.h>                    /* exit */


static int string_output(void *extra, int ch)
{
  /* the 'extra' argument is a pointer to a pointer to the
   * next character to write */
  char **s = (char**)extra;

  **s = ch;     /* write */
  (*s)++;       /* advance */

  return 0;
}

static int general_vsprintf(char *dest, char const *format, va_list args)
{
  char *s = dest;
  int ret;

  ret = general_vprintf(string_output, &s, format, args);
  *s = 0;

  return ret;
}


static char output1[1024];    /* for libc */
static char output2[1024];    /* for this module */


static void expect_vector_len(int expect_len, char const *expect_output,
                              char const *format, va_list args)
{
  int len;
  static int vectors = 0;

  /* keep track of how many vectors we've tried, to make it
   * a little easier to correlate failures with the inputs
   * in this file */
  vectors++;

  /* run the generalized vsprintf */
  len = general_vsprintf(output2, format, args);

  /* compare */
  if (len!=expect_len ||
      0!=strcmp(expect_output, output2)) {
    printf("outputs differ for vector %d!\n", vectors);
    printf("  format: %s\n", format);
    printf("  expect: %s (%d)\n", expect_output, expect_len);
    printf("      me: %s (%d)\n", output2, len);
    exit(2);
  }
}


static void expect_vector(char const *expect_output,
                          char const *format, ...)
{
  va_list args;
  va_start(args, format);
  expect_vector_len(strlen(expect_output), expect_output, format, args);
  va_end(args);
}


static void vector(char const *format, ...)
{
  va_list args;
  int len;

  /* run the real vsprintf */
  va_start(args, format);
  len = vsprintf(output1, format, args);
  va_end(args);

  /* test against the generalized vsprintf */
  va_start(args, format);
  expect_vector_len(len, output1, format, args);
  va_end(args);
}


/* Called from unit-tests.cc. */
void test_gprintf()
{
  /* test against libc */
  vector("simple");
  vector("a %s more", "little");
  vector("some %4d more %s complicated %c stuff",
         33, "yikes", 'f');

  /* test unknown format chars */
  expect_vector("XXXXXXXXXXXXXXXXXXXXXXXXXX", "%f", 3.4);
  expect_vector("XXXXXXXXXXXXXXXXXXXXXXX", "%.3f", 3.4);
  expect_vector("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "%.10f", 3.4);
  expect_vector("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "%.30f", 3.4);

  /* fails assertion, as it should */
  /* expect_vector("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "%.31f", 3.4); */

  /* TODO: add more tests */
}


/* EOF */
