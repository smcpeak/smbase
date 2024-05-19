// crc.h            see license.txt for copyright and terms of use
// simple crc function

#ifndef SMBASE_CRC_H
#define SMBASE_CRC_H

#include <stdint.h>                    // uint32_t

// Return the CRC32, as defined in this module, of the 'length' bytes
// pointed to by 'data'.
uint32_t crc32(unsigned char const *data, int length);

#endif // SMBASE_CRC_H

