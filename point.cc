// point.cc            see license.txt for copyright and terms of use
// code for point.h

#include "point.h"      // this module
#include "str.h"        // stringBuilder

#include <iostream>     // std::ostream


stringBuilder& operator<< (stringBuilder &sb, point const &pt)
{
  return sb << "(" << pt.x << ", " << pt.y << ")";
}

stringBuilder& operator<< (stringBuilder &sb, fpoint const &pt)
{
  return sb << "(" << pt.x << ", " << pt.y << ")";
}


std::ostream& operator<< (std::ostream &sb, point const &pt)
{
  return sb << "(" << pt.x << ", " << pt.y << ")";
}

std::ostream& operator<< (std::ostream &sb, fpoint const &pt)
{
  return sb << "(" << pt.x << ", " << pt.y << ")";
}


// EOF
