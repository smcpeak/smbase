// crc-test.cc
// Tests for crc module.

#include "crc.h"                       // module under test

#include "sm-test.h"                   // tprintf

#include <assert.h>                    // assert
#include <errno.h>                     // errno
#include <stdint.h>                    // uint32_t
#include <stdio.h>                     // FILE, etc.
#include <stdlib.h>                    // malloc, exit
#include <string.h>                    // strerror


static int errors=0;

static void testCrc(unsigned char const *data, int length, uint32_t crc)
{
  uint32_t val = crc32(data, length);
  tprintf("computed crc is 0x%08lX, expected is 0x%08lX\n",
          (unsigned long)val,
          (unsigned long)~crc);       // why is 'crc' inverted?
  if (val != ~crc) {
    errors++;
  }
}


// Called from unit-tests.cc.
void test_crc()
{
  if (char const *fname = getenv("TEST_CRC_FNAME")) {
    FILE *fp = fopen(fname, "r");
    if (!fp) {
      tprintf("error opening %s: %s\n", fname, strerror(errno));
      exit(2);
    }

    // get length
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    assert(len >= 0);
    fseek(fp, 0, SEEK_SET);

    // read the entire contents
    unsigned char *buf = (unsigned char*)malloc(len);
    if (fread(buf, 1, len, fp) != (size_t)len) {
      tprintf("read error, or short count..\n");
      exit(2);
    }

    // crc it
    long val = crc32(buf, len);
    tprintf("crc32 of %s: 0x%08lX\n", fname, val);

    return;
  }

  /* 40 Octets filled with "0" */
  /* CPCS-UU = 0, CPI = 0, Length = 40, CRC-32 = 864d7f99 */
  unsigned
  char pkt_data1[48]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                      0x00,0x00,0x00,0x28,0x86,0x4d,0x7f,0x99};

  /* 40 Octets filled with "1" */
  /* CPCS-UU = 0, CPI = 0, Length = 40, CRC-32 = c55e457a */
  unsigned
  char pkt_data2[48]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0x00,0x00,0x00,0x28,0xc5,0x5e,0x45,0x7a};

  /* 40 Octets counting: 1 to 40 */
  /* CPCS-UU = 0, CPI = 0, Length = 40, CRC-32 = bf671ed0 */
  unsigned
  char pkt_data3[48]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
                      0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
                      0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
                      0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
                      0x00,0x00,0x00,0x28,0xbf,0x67,0x1e,0xd0};

  /* 40 Octets counting: 1 to 40 */
  /* CPCS-UU = 11, CPI = 22, CRC-32 = acba602a */
  unsigned
  char pkt_data4[48]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
                      0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
                      0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
                      0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
                      0x11,0x22,0x00,0x28,0xac,0xba,0x60,0x2a};

  testCrc(pkt_data1, 44, 0x864d7f99);
  testCrc(pkt_data2, 44, 0xc55e457a);
  testCrc(pkt_data3, 44, 0xbf671ed0);
  testCrc(pkt_data4, 44, 0xacba602a);

  if (errors) {
    exit(2);
  }
}


// EOF
