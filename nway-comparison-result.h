// nway-comparison-result.h
// `NWayComparisonResult`, which holds the result of an N-way comparison.

// See license.txt for copyright and terms of use.

// All of the logic is in a header file so it can be inlined and then
// further optimized, as this is expected to be used within an inner
// loop.

#ifndef SMBASE_NWAY_COMPARISON_RESULT_H
#define SMBASE_NWAY_COMPARISON_RESULT_H

#include "nway-comparison-result-iface.h"        // interface for this module

#include "smbase/sm-macros.h"                    // DMEMB
#include "smbase/xassert.h"                      // xassert

#include <utility>                               // std::pair


OPEN_NAMESPACE(smbase)


NWayComparisonResult::NWayComparisonResult()
  : m_index1(-1),
    m_index2(-1)
{}


NWayComparisonResult::NWayComparisonResult(int smallestIndex)
  : m_index1(smallestIndex),
    m_index2(-1)
{
  xassert(smallestIndex >= 0);
}


NWayComparisonResult::NWayComparisonResult(int eqIndex1, int eqIndex2)
  : m_index1(eqIndex1),
    m_index2(eqIndex2)
{
  xassert(eqIndex1 >= 0);
  xassert(eqIndex2 >= 0);
  xassert(eqIndex1 != eqIndex2);
}


NWayComparisonResult::NWayComparisonResult(NWayComparisonResult const &obj)
  : DMEMB(m_index1),
    DMEMB(m_index2)
{}


NWayComparisonResult &NWayComparisonResult::operator=(NWayComparisonResult const &obj)
{
  if (this != &obj) {
    CMEMB(m_index1);
    CMEMB(m_index2);
  }
  return *this;
}


bool NWayComparisonResult::operator==(NWayComparisonResult const &obj)
{
  return EMEMB(m_index1) &&
         EMEMB(m_index2);
}


bool NWayComparisonResult::operator!=(NWayComparisonResult const &obj)
{
  return !operator==(obj);
}


bool NWayComparisonResult::isNull() const
{
  return m_index1 == -1;
}


bool NWayComparisonResult::hasSmallest() const
{
  return m_index1 >= 0 && m_index2 == -1;
}


bool NWayComparisonResult::hasEqualIndices() const
{
  return m_index1 >= 0 && m_index2 >= 0;
}


int NWayComparisonResult::getSmallest() const
{
  xassert(hasSmallest());
  return m_index1;
}


std::pair<int, int> NWayComparisonResult::getEqualIndices() const
{
  xassert(hasEqualIndices());
  return std::make_pair(m_index1, m_index2);
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_NWAY_COMPARISON_RESULT_H
