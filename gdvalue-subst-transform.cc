// gdvalue-subst-transform.cc
// Code for `gdvalue-subst-transform` module.

#include "gdvalue-subst-transform.h"   // this module

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/gdvsymbol.h"          // gdv::GDVSymbol
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP

#include <map>                         // std::map


OPEN_NAMESPACE(gdv)


GDValueSubstitutionTransform::~GDValueSubstitutionTransform()
{}


GDValueSubstitutionTransform::GDValueSubstitutionTransform(
  std::map<GDValue, GDValue> const &substitutions,
  bool recursive,
  std::map<GDVSymbol, GDVSymbol> const &containerTagSubstitutions)
:
  IMEMBFP(substitutions),
  IMEMBFP(recursive),
  IMEMBFP(containerTagSubstitutions)
{}


GDValue GDValueSubstitutionTransform::transform(GDValue const &v)
{
  auto it = m_substitutions.find(v);
  if (it != m_substitutions.end()) {
    GDValue ret((*it).second);
    if (m_recursive) {
      // Don't recursively apply
      // `GDValueSubstitutionTransform::transform` since that could lead
      // to an infinite loop due to something simple like mapping a
      // value to itself, or a pair of values that map to each other
      // (like a deep swap).
      //
      // Even when calling the base class method, this can still go into
      // an infinite loop if the transformed value contains its own key.
      ret = GDValueTransform::transform(ret);
    }
    return ret;
  }
  else {
    return GDValueTransform::transform(v);
  }
}


GDVSymbol GDValueSubstitutionTransform::transformContainerTag(GDVSymbol tag)
{
  auto it = m_containerTagSubstitutions.find(tag);
  if (it != m_containerTagSubstitutions.end()) {
    return (*it).second;
  }
  else {
    return tag;
  }
}


GDValue substitutionTransformGDValue(
  GDValue const &input,
  std::map<GDValue, GDValue> const &substitutions,
  bool recursive,
  std::map<GDVSymbol, GDVSymbol> const &containerTagSubstitutions)
{
  GDValueSubstitutionTransform transform(
    substitutions,
    recursive,
    containerTagSubstitutions);
  return transform.transform(input);
}


CLOSE_NAMESPACE(gdv)


// EOF
