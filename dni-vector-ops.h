// dni-vector-ops.h
// Extra operations for `dni-vector.h`.

#ifndef SMBASE_DNI_VECTOR_OPS_H
#define SMBASE_DNI_VECTOR_OPS_H

#include "smbase/dni-vector.h"         // this module

#include "smbase/gdvalue-vector.h"     // gdv::toGDValue(std::vector)
#include "smbase/gdvalue.h"            // gdv::GDValue

#include <utility>                     // std::move
#include <vector>                      // std::vector


OPEN_NAMESPACE(smbase)


template <typename INDEX, typename VALUE>
DNIVector<INDEX,VALUE>::DNIVector(DNIVector &&obj)
  : MDMEMB(m_vec)
{}


template <typename INDEX, typename VALUE>
DNIVector<INDEX,VALUE> &DNIVector<INDEX,VALUE>::operator=(DNIVector &&obj)
{
  MCMEMB(m_vec);
  return *this;
}


template <typename INDEX, typename VALUE>
void DNIVector<INDEX,VALUE>::push_back(VALUE &&value)
{
  m_vec.push_back(std::move(value));
}


template <typename INDEX, typename VALUE>
DNIVector<INDEX,VALUE>::operator gdv::GDValue() const
{
  using gdv::toGDValue;
  return toGDValue(m_vec);
}


template <typename INDEX, typename VALUE>
void DNIVector<INDEX,VALUE>::write(std::ostream &os) const
{
  os << operator gdv::GDValue();
}


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_DNI_VECTOR_OPS_H
