// gdvtuple.cc
// Code for `gdvtuple` module.

#include "gdvtuple.h"                  // this module

#include "compare-util.h"              // COMPARE_MEMBERS
#include "gdvalue.h"                   // GDValue
#include "sm-macros.h"                 // DMEMB

#include <utility>                     // std::move


OPEN_NAMESPACE(gdv)


GDVTuple::~GDVTuple()
{}


GDVTuple::GDVTuple() noexcept
  : m_vector()
{}


GDVTuple::GDVTuple(size_type count, GDValue const &value)
  : m_vector(count, value)
{}


GDVTuple::GDVTuple(size_type count)
  : m_vector(count)
{}


GDVTuple::GDVTuple(GDVTuple const &obj)
  : DMEMB(m_vector)
{}


GDVTuple::GDVTuple(GDVTuple &&obj)
  : MDMEMB(m_vector)
{}


GDVTuple::GDVTuple(std::initializer_list<GDValue> init)
  : m_vector(init)
{}


GDVTuple &GDVTuple::operator=(GDVTuple const &obj)
{
  if (this != &obj) {
    CMEMB(m_vector);
  }
  return *this;
}


GDVTuple &GDVTuple::operator=(GDVTuple &&obj) noexcept
{
  if (this != &obj) {
    MCMEMB(m_vector);
  }
  return *this;
}


GDVTuple &GDVTuple::operator=(std::initializer_list<GDValue> init)
{
  m_vector = init;
  return *this;
}


GDValue const &GDVTuple::at(size_type pos) const
{
  return m_vector.at(pos);
}


GDValue &GDVTuple::at(size_type pos)
{
  return m_vector.at(pos);
}


GDValue const &GDVTuple::operator[](size_type pos) const
{
  return m_vector[pos];
}


GDValue &GDVTuple::operator[](size_type pos)
{
  return m_vector[pos];
}


GDVTuple::const_iterator GDVTuple::cbegin() const noexcept
{
  return m_vector.cbegin();
}


GDVTuple::const_iterator GDVTuple::begin() const
{
  return m_vector.begin();
}


GDVTuple::iterator GDVTuple::begin()
{
  return m_vector.begin();
}


GDVTuple::const_iterator GDVTuple::cend() const noexcept
{
  return m_vector.end();
}


GDVTuple::const_iterator GDVTuple::end() const noexcept
{
  return m_vector.end();
}


GDVTuple::iterator GDVTuple::end() noexcept
{
  return m_vector.end();
}


bool GDVTuple::empty() const noexcept
{
  return m_vector.empty();
}


GDVTuple::size_type GDVTuple::size() const noexcept
{
  return m_vector.size();
}


void GDVTuple::clear() noexcept
{
  m_vector.clear();
}


GDVTuple::iterator GDVTuple::insert(const_iterator pos, GDValue const &value)
{
  return m_vector.insert(pos, value);
}


GDVTuple::iterator GDVTuple::erase(const_iterator pos)
{
  return m_vector.erase(pos);
}


void GDVTuple::push_back(GDValue const &value)
{
  m_vector.push_back(value);
}


void GDVTuple::push_back(GDValue &&value)
{
  m_vector.push_back(std::move(value));
}


void GDVTuple::resize(size_type count)
{
  m_vector.resize(count);
}


void GDVTuple::swap(GDVTuple &obj) noexcept
{
  m_vector.swap(obj.m_vector);
}


int compare(GDVTuple const &a, GDVTuple const &b)
{
  return COMPARE_MEMBERS(m_vector);
}


CLOSE_NAMESPACE(gdv)


// EOF
