// crc.cc            see license.txt for copyright and terms of use
// adapted slightly by Scott McPeak

#include "crc.h"                       // this module

#include <stdint.h>                    // uint32_t

// originally was:
/* crc32h.c -- package to compute 32-bit CRC one byte at a time using   */
/*             the high-bit first (Big-Endian) bit ordering convention  */
/*                                                                      */
/* Synopsis:                                                            */
/*  gen_crc_table() -- generates a 256-word table containing all CRC    */
/*                     remainders for every possible 8-bit byte.  It    */
/*                     must be executed (once) before any CRC updates.  */
/*                                                                      */
/*  unsigned update_crc(crc_accum, data_blk_ptr, data_blk_size)         */
/*           unsigned crc_accum; char *data_blk_ptr; int data_blk_size; */
/*           Returns the updated value of the CRC accumulator after     */
/*           processing each byte in the addressed block of data.       */
/*                                                                      */
/*  It is assumed that an unsigned long is at least 32 bits wide and    */
/*  that the predefined type char occupies one 8-bit byte of storage.   */
/*                                                                      */
/*  The generator polynomial used for this version of the package is    */
/*  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0 */
/*  as specified in the Autodin/Ethernet/ADCCP protocol standards.      */
/*  Other degree 32 polynomials may be substituted by re-defining the   */
/*  symbol POLYNOMIAL below.  Lower degree polynomials must first be    */
/*  multiplied by an appropriate power of x.  The representation used   */
/*  is that the coefficient of x^0 is stored in the LSB of the 32-bit   */
/*  word and the coefficient of x^31 is stored in the most significant  */
/*  bit.  The CRC is to be appended to the data most significant byte   */
/*  first.  For those protocols in which bytes are transmitted MSB      */
/*  first and in the same order as they are encountered in the block    */
/*  this convention results in the CRC remainder being transmitted with */
/*  the coefficient of x^31 first and with that of x^0 last (just as    */
/*  would be done by a hardware shift register mechanization).          */
/*                                                                      */
/*  The table lookup technique was adapted from the algorithm described */
/*  by Avram Perez, Byte-wise CRC Calculations, IEEE Micro 3, 40 (1983).*/

#define POLYNOMIAL 0x04c11db7L

static uint32_t crc_table[256];

void gen_crc_table()
 /* generate the table of CRC remainders for all possible bytes */
 { int i, j;  uint32_t crc_accum;
   for ( i = 0;  i < 256;  i++ )
       { crc_accum = ( (uint32_t) i << 24 );
         for ( j = 0;  j < 8;  j++ )
              { if ( crc_accum & 0x80000000L )
                   crc_accum =
                     ( crc_accum << 1 ) ^ POLYNOMIAL;
                else
                   crc_accum =
                     ( crc_accum << 1 ); }
         crc_table[i] = crc_accum; }
   return; }

uint32_t update_crc(uint32_t crc_accum, char const *data_blk_ptr,
                                        int data_blk_size)
 /* update the CRC on the data block one byte at a time */
 { int i, j;
   for ( j = 0;  j < data_blk_size;  j++ )
       { i = ( (int) ( crc_accum >> 24) ^ *data_blk_ptr++ ) & 0xff;
         crc_accum = ( crc_accum << 8 ) ^ crc_table[i]; }
   return crc_accum; }


// SM: block-level application
static int made_table = 0;
uint32_t crc32(unsigned char const *data, int length)
{
  if (!made_table) {
    gen_crc_table();
    made_table = 1;
  }

  return update_crc(0xFFFFFFFF, (char*)data, length);
}


// EOF
