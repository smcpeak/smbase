// nway-comparison-result-iface.h
// Interface for `nway-comparison-result` module

// See license.txt for copyright and terms of use.

#ifndef SMBASE_NWAY_COMPARISON_RESULT_IFACE_H
#define SMBASE_NWAY_COMPARISON_RESULT_IFACE_H

#include "nway-comparison-result-fwd.h"          // fwd decls for this module

#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE
#include "smbase/std-utility-fwd.h"              // std::pair


OPEN_NAMESPACE(smbase)


// Hold the result of comparing a sequence of N things to find the
// smallest.  It is a disjoint union of:
//
//   - null result
//   - int
//   - pair(int, int)
//
class NWayComparisonResult {
private:     // data
  // -1 for a null result; otherwise either `smallestIndex` or
  // `eqIndex1`.
  int m_index1;

  // -1 for a null result or single int; otherwise, `eqIndex2`.
  int m_index2;

public:      // methods
  // Null result, there was nothing to compare (N is 0 or 1).
  inline explicit NWayComparisonResult();

  // The smallest was `smallestIndex`, which must be in [0, N-1].  Here,
  // `N` refers to the number of items compared, but this class does not
  // actually store that value, so cannot fully enforce the
  // precondition.
  inline explicit NWayComparisonResult(int smallestIndex);

  // At least two elements were equal, and these are their indices,
  // which must be distinct and in [0, N-1].  Depending on the algorithm
  // that finds them, these are *not* necessarily the smallest values
  // that are equal.
  inline explicit NWayComparisonResult(int eqIndex1, int eqIndex2);

  inline NWayComparisonResult(NWayComparisonResult const &obj);
  inline NWayComparisonResult &operator=(NWayComparisonResult const &obj);

  inline bool operator==(NWayComparisonResult const &obj);
  inline bool operator!=(NWayComparisonResult const &obj);

  // Which of the three possibilities this is.
  inline bool isNull() const;
  inline bool hasSmallest() const;
  inline bool hasEqualIndices() const;

  // Get the smallest index.
  //
  // Requires: hasSmallest()
  inline int getSmallest() const;

  // Get the pair of equal indices.
  //
  // Requires: hasEqualIndices()
  inline std::pair<int, int> getEqualIndices() const;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_NWAY_COMPARISON_RESULT_IFACE_H
