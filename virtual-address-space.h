// virtual-address-space.h
// `VirtualASManager`, a mapping between 2D virtual and 1D global
// addresses.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_VIRTUAL_ADDRESS_SPACE_H
#define SMBASE_VIRTUAL_ADDRESS_SPACE_H

#include "virtual-address-space-fwd.h" // fwds for this module

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [n]
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/std-utility-fwd.h"    // std::pair [n]
#include "smbase/sum-tree-fwd.h"       // smbase::SumTree [n]

#include <memory>                      // std::unique_ptr
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


/* Map between a global address space and multiple local virtual address
   spaces.

   An "address space" is a contiguous finite set of natural numbers
   starting with 0, e.g., [0,99].

   Each virtual address space exclusively occupies some potentially
   non-contiguous portion of the global space.  A virtual address space
   can be extended at any time (which leads to non-contiguous
   allocation).

   As a simple example, we might have a global address space that looks
   like:

     [0 A 9][0 B 49][10 A 79][0 C 999]
     ^      ^       ^        ^        ^
     0      10      60       130      1130

   The global space is [0,1129].  It is partitioned across three virtual
   address spaces, A, B, and C, as follows:

     * [0,9] maps to [0,9] in virtual address A.
     * [10,59] maps to [0,49] in B.
     * [60,129] maps to [10,79] in A.
     * [130,1129] maps to [0,999] in C.

   The purpose is to allow a single number, the global address, to
   compactly encode two numbers, the virtual address space and the
   offset within that space.  A balanced tree data structure provides
   logarithmic-time mapping between the two.

   One application is representing locations in a collection of files.
   Rather than storing a pair, one can store a single number, which can
   be an important advantage if many objects carry a location.  The
   ability to extend address spaces is important because the program may
   be reading multiple files at once (e.g., processing #include
   directives) and not know in advance how big each is.
*/
class VirtualASManager {
public:      // types
  // Virtual Address Space identifier.
  using VASID = int;

  // Offset into the global address space.
  using GlobalOffset = int;

  // Offset into a local address space.
  using LocalOffset = int;

private:     // types
  // Element stored in the tree.
  class VASFragment {
  public:      // types
    using Summary = GlobalOffset;

  public:      // data
    // Which local space this fragment is a part of.
    VASID m_vas;

    // Where in the local space does this fragment start?
    //
    // Always non-negative.
    LocalOffset m_start;

    // Size of the fragment in both local and global space.
    //
    // Always non-negative.
    LocalOffset m_size;

  public:      // methods
    // The summary is `m_size`.
    GlobalOffset summary() const;

    // Check invariants.
    void selfCheck() const;

    // Dump fields.
    operator gdv::GDValue() const;
  };

private:     // data
  // Tree of local fragments.
  //
  // Invariant: For each `vas`, the sequence of elements with that ID
  // describes a contiguous space: each `m_start` is the sum of all the
  // `m_size`s that preceded it (with that ID).
  std::unique_ptr<SumTree<VASFragment>> m_tree;

  // Map from VASID to its allocated size.
  //
  // Invariant: For each valid index `vas`, its value in `m_vasSizes`
  // equals the sum of `m_size` fields in `m_tree` where `m_vas==vas`.
  std::vector<LocalOffset> m_vasSizes;

public:      // methods
  ~VirtualASManager();

  // Empty global space, no virtual address spaces.
  VirtualASManager();

  // Assert invariants.
  void selfCheck() const;

  // ----------------------------- Queries -----------------------------
  // Total size of the global address space.
  GlobalOffset globalSpaceSize() const;

  // Number of local address spaces.
  VASID numLocalSpaces() const;

  // True if `vas` names an allocated space.
  //
  // Returns: bool(0 <= vas < numLocalSpaces())
  bool validLocalSpace(VASID vas) const;

  // Size of local space `vasid`.
  //
  // Requires: validLocalSpace(vas)
  LocalOffset localSpaceSize(VASID vas) const;

  // Look up global `offset` and return its associated virtual address
  // space and the offset within that space.
  //
  // Requires: 0 <= offset < globalSpaceSize()
  std::pair<VASID, LocalOffset> globalToLocal(GlobalOffset offset) const;

  // -------------------------- Modification ---------------------------
  // Allocate a new local space.  Does not allocate any space inside it.
  //
  // Returns: pre(numLocalSpaces())
  // Ensures: numLocalSpaces() == return + 1
  // Ensures: localSpaceSize(return) == 0
  VASID allocateLocalSpace();

  // Extend `vas` by `size`.  Returns the start of the newly allocated
  // region of the global space that is allocated to `vas`.
  //
  // Requires: validLocalSpace(vas)
  // Requires: size >= 0
  //
  // Returns: pre(globalSpaceSize())
  // Ensures: globalSpaceSize() == return + size
  // Ensures: localSpaceSize(vas) == pre(localSpaceSize(vas)) + size
  GlobalOffset extendLocalSpace(VASID vas, LocalOffset size);
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_VIRTUAL_ADDRESS_SPACE_H
