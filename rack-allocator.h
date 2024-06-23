// rack-allocator.h
// Allocate small objects contiguously.  Does not allow deallocation of
// individual objects.

// This file is in the public domain.

// The implementation is partly based on the internals of `strtable.h`.

#ifndef SMBASE_RACK_ALLOCATOR_H
#define SMBASE_RACK_ALLOCATOR_H

#include "sm-macros.h"                 // OPEN_NAMESPACE, NULLABLE

#include <cstddef>                     // std::{ptrdiff_t, size_t}
#include <iosfwd>                      // std::ostream


OPEN_NAMESPACE(smbase)


// Allocate small objects contiguously.  Does not allow deallocation of
// individual objects.
class RackAllocator {
  // For now.
  NO_OBJECT_COPIES(RackAllocator);

public:      // types
  enum {
    RACK_SIZE = 16000,       // Size of one rack.
    LARGE_THRESHOLD = 1000,  // Minimum length of a "large" allocation.
  };

  // The type of object allocated.  As far as this class is concerned,
  // it is simply allocating bytes.
  typedef unsigned char value_type;

  // Sizes of allocated objects.
  typedef std::size_t size_type;

  // Result of pointer subtraction.
  typedef std::ptrdiff_t difference_type;

private:     // types
  // Holds allocated small objects.
  struct Rack {
    // Nullable owner pointer to the next rack in the list, which is
    // used when we deallocate everything.
    Rack * NULLABLE m_next;

    // The number of bytes of `data` that are currently used.
    //
    // Invariant: `0 <= m_usedBytes && m_usedBytes <= RACK_SIZE`.
    size_type m_usedBytes;

    // Storage for allocated objects.
    unsigned char m_data[RACK_SIZE];

  public:
    Rack(Rack *next)
      : m_next(next),
        m_usedBytes(0),
        m_data()
    {}

    // Number of bytes that are available in this rack.
    size_type availBytes() const
      { return RACK_SIZE - m_usedBytes; }

    // Pointer to the next available byte.
    unsigned char *nextByte()
      { return m_data + m_usedBytes; }
  };

  // Stores large allocations.
  struct LargeBlock {
    // Nullable owner pointer to the next in the list, used for
    // deallocation.
    LargeBlock * NULLABLE m_next;

    // The allocated data is stored after this object.
  };

private:     // data
  // Pointer to the rack we are currently filling.  When it is full, we
  // make a new one and make it the head element, with its `m_next`
  // pointing to the old head.
  Rack * NULLABLE m_firstRack;

  // Pointer to the most-recently allocated large block, which has a
  // pointer to the next-most-recent, etc.
  LargeBlock * NULLABLE m_firstLarge;

public:      // methods
  ~RackAllocator();
  RackAllocator();

  // Allocate `n` bytes, aligned to a pointer boundary.
  unsigned char *allocate(std::size_t n);

  // Deallocate all objects at once.
  void clear();

  // Print test/performance stats.
  void printStats(std::ostream &os) const;

  // Assert invariants.
  void selfCheck() const;

  // ---- Statistics for testing and performance evaluation

  // Number of allocated Racks
  int numRacks() const;

  // Number of allocated LargeBlocks.
  int numLargeBlocks() const;

  // Total amount of unused space that is in allocated rack objects that
  // are not the first anymore.
  std::size_t wastedSpace() const;

  // Amount of available space in the first rack.
  std::size_t availSpaceInFirstRack() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_RACK_ALLOCATOR_H
