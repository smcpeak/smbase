// gdvalue-subst-transform.h
// `GDValueSubstitutionTransform`, a substitution-based transform.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_SUBST_TRANSFORM_H
#define SMBASE_GDVALUE_SUBST_TRANSFORM_H

#include "gdvalue-subst-transform-fwd.h"         // fwds for this module

#include "smbase/gdv-symbol-fwd.h"               // gdv::GDVSymbol [n]
#include "smbase/gdvalue-fwd.h"                  // gdv::GDValue [n]
#include "smbase/gdvalue-transform.h"            // gdv::GDValueTransform
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE

#include <map>                                   // std::map


OPEN_NAMESPACE(gdv)


// Substitute according to a specified map while transforming.
class GDValueSubstitutionTransform : public GDValueTransform {
public:      // data
  // Every input value that matches a key will be replaced with the
  // corresponding value.
  std::map<GDValue, GDValue> m_substitutions;

  // If true, then after a substitution is performed, recursively apply
  // substitutions to that value.
  bool m_recursive;

  // Map of container tag substitutions to perform.
  std::map<GDVSymbol, GDVSymbol> m_containerTagSubstitutions;

public:
  ~GDValueSubstitutionTransform();

  GDValueSubstitutionTransform(
    std::map<GDValue, GDValue> const &substitutions = {},
    bool recursive = false,
    std::map<GDVSymbol, GDVSymbol> const &containerTagSubstitutions = {});

  // If `v` is a key in `m_substitutions`, then return its value,
  // possibly after recursively substituting.  Otherwise return `v` as
  // transformed by the base class.
  virtual GDValue transform(GDValue const &v) override;

  // If `tag` is a key in `m_containerTagSubstitutions`, return its
  // value.  Otherwise return `tag`.
  virtual GDVSymbol transformContainerTag(GDVSymbol tag) override;
};


// Run the substitution transformation.
GDValue substitutionTransformGDValue(
  GDValue const &input,
  std::map<GDValue, GDValue> const &substitutions,
  bool recursive = false,
  std::map<GDVSymbol, GDVSymbol> const &containerTagSubstitutions = {});


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_SUBST_TRANSFORM_H
