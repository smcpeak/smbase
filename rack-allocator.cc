// rack-allocator.cc
// Code for `rack-allocator` module.

// This file is in the public domain.

#include "rack-allocator.h"            // this module

#include "div-up.h"                    // round_up
#include "sm-macros.h"                 // OPEN_NAMESPACE
#include "sm-test.h"                   // PVALTO
#include "xassert.h"                   // xassertPrecondition

#include <iostream>                    // std::ostream


OPEN_NAMESPACE(smbase)


RackAllocator::~RackAllocator()
{
  clear();
}


RackAllocator::RackAllocator()
  : m_firstRack(nullptr),
    m_firstLarge(nullptr)
{}


unsigned char *RackAllocator::allocate(std::size_t n)
{
  // Round `n` up to the next pointer boundary.
  n = round_up(n, sizeof(void*));

  if (n >= LARGE_THRESHOLD) {
    // We need space for our `LargeBlock` plus the caller's `n` bytes.
    std::size_t fullSize = n + sizeof(LargeBlock);

    // If this fails, the arithmetic overflowed, so `n` was unreasonably
    // large.
    xassertPrecondition(fullSize > n);

    // Allocate.
    unsigned char *fullData = new unsigned char[fullSize];
    LargeBlock *block = reinterpret_cast<LargeBlock*>(fullData);

    // Prepend `block` to the list.
    block->m_next = m_firstLarge;
    m_firstLarge = block;

    return fullData + sizeof(LargeBlock);
  }

  if (m_firstRack == nullptr ||
      m_firstRack->availBytes() < n) {
    // Need a new rack.
    m_firstRack = new Rack(m_firstRack);
  }

  // Grab space from the first rack.
  unsigned char *ret = m_firstRack->nextByte();
  m_firstRack->m_usedBytes += n;
  return ret;
}


void RackAllocator::clear()
{
  while (m_firstRack != nullptr) {
    Rack *next = m_firstRack->m_next;
    delete m_firstRack;
    m_firstRack = next;
  }

  while (m_firstLarge != nullptr) {
    LargeBlock *next = m_firstLarge->m_next;
    delete[] reinterpret_cast<unsigned char *>(m_firstLarge);
    m_firstLarge = next;
  }
}


void RackAllocator::printStats(std::ostream &os) const
{
  PVALTO(os, numRacks());
  PVALTO(os, numLargeBlocks());
  PVALTO(os, wastedSpace());
  PVALTO(os, availSpaceInFirstRack());
}


void RackAllocator::selfCheck() const
{
  // Nothing to check at this time.
}


int RackAllocator::numRacks() const
{
  int ret = 0;
  for (Rack const *r = m_firstRack; r; r = r->m_next) {
    ++ret;
  }
  return ret;
}


int RackAllocator::numLargeBlocks() const
{
  int ret = 0;
  for (LargeBlock const *lb = m_firstLarge; lb; lb = lb->m_next) {
    ++ret;
  }
  return ret;
}


std::size_t RackAllocator::wastedSpace() const
{
  std::size_t ret = 0;
  for (Rack const *r = m_firstRack; r; r = r->m_next) {
    if (r != m_firstRack) {
      ret += r->availBytes();
    }
  }
  return ret;
}


std::size_t RackAllocator::availSpaceInFirstRack() const
{
  if (m_firstRack) {
    return m_firstRack->availBytes();
  }
  else {
    return 0;
  }
}


CLOSE_NAMESPACE(smbase)


// EOF
