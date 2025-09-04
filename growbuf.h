// growbuf.h            see license.txt for copyright and terms of use
// Extension of DataBlock that provides an append() function.

#ifndef SMBASE_GROWBUF_H
#define SMBASE_GROWBUF_H

#include "datablok.h"                  // DataBlock

#include <cstddef>                     // std::size_t

class GrowBuffer : public DataBlock {
public:
  GrowBuffer(std::size_t allocSize=0)
    : DataBlock(allocSize) {}
  ~GrowBuffer() {}

  // append to the end, at least doubling allocated
  // size if growth is needed
  void append(unsigned char const *str, std::size_t len);
  void append(char const *str, std::size_t len)
    { append((unsigned char const*)str, len); }
};

#endif // SMBASE_GROWBUF_H
