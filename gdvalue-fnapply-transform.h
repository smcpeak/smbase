// gdvalue-fnapply-transform.h
// `GDValueFnApplyTransform`, a transform with function application
// capability, thus providing a basic computation model.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_FNAPPLY_TRANSFORM_H
#define SMBASE_GDVALUE_FNAPPLY_TRANSFORM_H

#include "gdvalue-fnapply-transform-fwd.h"       // fwds for this module

#include "smbase/gdv-symbol.h"                   // gdv::GDVSymbol
#include "smbase/gdvalue-parser-fwd.h"           // gdv::GDValueParser [n]
#include "smbase/gdvalue-transform.h"            // gdv::GDValueTransform
#include "smbase/gdvalue.h"                      // gdv::{GDValue, GDVTaggedTuple}
#include "smbase/sm-macros.h"                    // OPEN_NAMESPACE, NO_OBJECT_COPIES
#include "smbase/std-functional-fwd.h"           // std::reference_wrapper [n]
#include "smbase/std-set-fwd.h"                  // std::set [n]

#include <cstddef>                               // std::size_t
#include <list>                                  // std::list
#include <map>                                   // std::map
#include <vector>                                // std::vector


OPEN_NAMESPACE(gdv)


/* Substitute and transform with function application.

   This class provides, in essence, a very simple computation model.  It
   has two special forms:

     * Variable definitions: Define(x 3)

     * Function definitions: Define(f(n) (n n))

   It then transforms the input in order.  Both special forms transform
   to null.  A use of a variable or function after its definition
   transforms to the defined value, with function application
   substituting arguments for values.

   Example input:

     [
       123
       Define(x 456)
       x
       Define(f(n) (n n))
       f(2)
       f(x)
     ]

   Corresponding output:

     [
       123         // Not a defined var/func, so self-evaluating.
       null        // Result of `Define`.
       456         // Transformation of defined variable `x`.
       null        // Result of `Define`.
       (2 2)       // Transformation of call to defined function `f`.
       (456 456)   // Same, but with a transformed argument.
     ]

   NOTE: Currently the only scoping is within function definition
   bodies.

   TODO: Improve the scoping.
*/
class GDValueFnApplyTransform : public GDValueTransform {
  // For now at least.
  NO_OBJECT_COPIES(GDValueFnApplyTransform);

private:     // types
  // Optional reference wrapper.
  template <typename T>
  using RefWrapOpt = std::optional<std::reference_wrapper<T>>;

  // The parameters and body of a function definition.  (The name is
  // stored separately, as a key in `m_functions`.
  class FnParamsAndBody {
  public:      // data
    // Parameters of ths function.
    std::vector<GDVSymbol> m_params;

    // Body in which to substitute parameters, plus any names defined in
    // outer scopes, when evaluating an application of this function.
    GDValue m_body;
  };

  // The names arising from one function application, or the global
  // names.
  //
  // Currently, variables and functions are in completely separate
  // namespaces.
  class ActivationRecord {
  public:      // data
    // Map from variable name to its bound value.
    std::map<GDVSymbol, GDValue> m_variables;

    // Map from function name to parameters and function body.
    std::map<GDVSymbol, FnParamsAndBody> m_functions;

  public:      // methods
    // Return the set of bound variable or function names.
    std::set<GDVSymbol> variableNames() const;
    std::set<GDVSymbol> functionNames() const;
  };

private:     // data
  // List of environments to search for names in, from innermost to
  // outermost.
  //
  // Invariant: !m_env.empty()
  std::list<ActivationRecord> m_env;

private:     // methods
  // Lookup name, returning nullopt if not found.
  RefWrapOpt<GDValue const> lookupVariableName(
    GDVSymbol name) const;
  RefWrapOpt<FnParamsAndBody const> lookupFunctionName(
    GDVSymbol name) const;

  // Get the innermost record.
  ActivationRecord const &innerARC() const;
  ActivationRecord &innerAR();

  // Handle a "Define" special form.
  GDValue handleDefine(GDValueParser const &parser);

  // Check that all of the symbols in `params` are unique.  If not,
  // throw `XGDValueError` using `defnParser`, the parser pointing at
  // the tagged tuple from which the parameters were extracted.
  void checkUniqueParameters(
    GDValueParser const &defnParser,
    std::vector<GDVSymbol> const &params);

  // Handle `appParser`, an application of `name`, which we have found
  // is bound to `paramsAndBody`.
  GDValue handleFunctionApplication(
    GDValueParser const &appParser,
    GDVSymbol name,
    FnParamsAndBody const &paramsAndBody);

public:      // methods
  ~GDValueFnApplyTransform();

  explicit GDValueFnApplyTransform();

  // Assert invariants.
  void selfCheck() const;

  /* If `v` is a special form, update the environment and return what
     the form evaluates to.

     If `v` is a key in `m_names`, return its value.

     If `v` matches a key in `m_funcs`, substitute the arguments for
     the corresponding parameters and return the result.

     If none of the above, return `v` after recursive transformation of
     its elements (if it is a container).

     On error, throws `XGDValueError`.
  */
  virtual GDValue transform(GDValue const &v) override;

  // Current depth of the environment, i.e., the number of activation
  // records in `m_env`.
  //
  // Ensures: return >= 1
  std::size_t environmentDepth() const;

  // Return the set of variable or function names bound in the innermost
  // activation record.
  std::set<GDVSymbol> innermostARVariableNames() const;
  std::set<GDVSymbol> innermostARFunctionNames() const;
};


// Create a `GDValueFnApplyTransform` and apply it to `v`.
GDValue fnApplyTransformGDValue(GDValue const &v);


CLOSE_NAMESPACE(gdv)


#endif // SMBASE_GDVALUE_FNAPPLY_TRANSFORM_H
