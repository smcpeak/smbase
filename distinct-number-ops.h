// distinct-number-ops.h
// Extra operations for `distinct-number`.

// In other modules in smbase, I have named the thin interface with
// "-iface.h" and kept the unsuffixed name for the ops.  Here, I'm
// experimenting with reversing that convention and adding "-ops.h" to
// the ops file.

#ifndef DISTINCT_NUMBER_OPS_H
#define DISTINCT_NUMBER_OPS_H

#include "distinct-number.h"           // this module

#include "gdvalue.h"                   // gdv::GDValue
#include "sm-macros.h"                 // OPEN_NAMESPACE

#include <iostream>                    // std::ostream


OPEN_NAMESPACE(smbase)


template <typename TAG, typename NUM>
void DistinctNumber<TAG,NUM>::write(std::ostream &os) const
{
  os << m_num;
}


template <typename TAG, typename NUM>
DistinctNumber<TAG,NUM>::operator gdv::GDValue() const
{
  using gdv::toGDValue;
  return toGDValue(m_num);
}


CLOSE_NAMESPACE(smbase)


#endif // DISTINCT_NUMBER_OPS_H
