// gdvalue-fnapply-transform.cc
// Code for `gdvalue-fnapply-transform` module.

#include "gdvalue-fnapply-transform.h" // this module

#include "smbase/exc.h"                // EXN_CONTEXT
#include "smbase/gdvalue.h"            // gdv::{GDValue, GDVTaggedTuple} [n]
#include "smbase/gdvalue-parser.h"     // gdv::GDValueParser
#include "smbase/gdvalue-transform.h"  // gdv::GDValueTransform
#include "smbase/gdvalue-tuple.h"      // gdv::gdvpTo<std::tuple>
#include "smbase/gdvalue-vector.h"     // gdv::gdvpTo<std::vector>
#include "smbase/map-util.h"           // smbase::mapKeySet
#include "smbase/set-util.h"           // smbase::setInsert
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP

#include <functional>                  // std::reference_wrapper
#include <set>                         // std::set
#include <tuple>                       // std::tuple
#include <utility>                     // std::move

using namespace smbase;


OPEN_NAMESPACE(gdv)


// ------------------------- ActivationRecord --------------------------
std::set<GDVSymbol>
GDValueFnApplyTransform::ActivationRecord::variableNames() const
{
  return mapKeySet(m_variables);
}


std::set<GDVSymbol>
GDValueFnApplyTransform::ActivationRecord::functionNames() const
{
  return mapKeySet(m_functions);
}


// ---------------------- GDValueFnApplyTransform ----------------------
GDValueFnApplyTransform::~GDValueFnApplyTransform()
{}


GDValueFnApplyTransform::GDValueFnApplyTransform()
:
  m_env()
{
  // Create the outermost record, initially empty.
  m_env.push_front(ActivationRecord());
}


void GDValueFnApplyTransform::selfCheck() const
{
  xassert(!m_env.empty());
}


// Return an optional reference wrapper to the value for a specified key
// in a map.
//
// TODO: Move to `map-util`.
template <typename K, typename V, typename C, typename A>
std::optional<std::reference_wrapper<V const>> mapGetValueAtOpt(
  std::map<K,V,C,A> const &m,
  K const &k)
{
  auto it = m.find(k);
  if (it != m.end()) {
    return std::make_optional(std::cref( (*it).second ));
  }
  else {
    return std::nullopt;
  }
}


auto GDValueFnApplyTransform::lookupVariableName(GDVSymbol name) const
  -> RefWrapOpt<GDValue const>
{
  // Search the environments in order.
  for (ActivationRecord const &ar : m_env) {
    if (auto valueOpt = mapGetValueAtOpt(ar.m_variables, name)) {
      return valueOpt;
    }
  }

  return std::nullopt;
}


auto GDValueFnApplyTransform::lookupFunctionName(GDVSymbol name) const
  -> RefWrapOpt<FnParamsAndBody const>
{
  for (ActivationRecord const &ar : m_env) {
    if (auto valueOpt = mapGetValueAtOpt(ar.m_functions, name)) {
      return valueOpt;
    }
  }

  return std::nullopt;
}


auto GDValueFnApplyTransform::innerARC() const
  -> ActivationRecord const &
{
  xassert(!m_env.empty());
  return m_env.front();
}


auto GDValueFnApplyTransform::innerAR() -> ActivationRecord &
{
  return const_cast<ActivationRecord &>(innerARC());
}


GDValue GDValueFnApplyTransform::handleDefine(
  GDValueParser const &parser)
{
  EXN_CONTEXT("Define");
  parser.checkContainerSize(2);

  auto [nameAndParams, body] =
    gdvpTo<std::tuple<GDValueParser, GDValue>>(parser);

  if (nameAndParams.isSymbol()) {
    GDVSymbol variableName = nameAndParams.symbolGet();
    EXN_CONTEXT(variableName);

    // Apply substitutions in the body before `name` is defined.
    //
    // Consequently, variables follow an "eager" evaluation model,
    // like GNU make ":=" definitions, as opposed to "=" definitions.
    GDValue substBody = transform(body);

    // Bind that to `name`.
    innerAR().m_variables[variableName] = substBody;
  }

  else if (nameAndParams.isTaggedTuple()) {
    GDVSymbol functionName = nameAndParams.taggedContainerGetTag();
    EXN_CONTEXT(functionName);

    // TODO: Provide a more convenient way to do this.
    // gdvpTo<std::vector> does not work because that insists on the
    // input being a sequence, and gdvpTo<GDVTuple> does not work
    // because that does not constrain the element types.
    //
    // Also, it would be nice to be able to iterate over a container
    // parser, getting element parsers.
    std::vector<GDVSymbol> params;
    for (GDVIndex i=0; i < nameAndParams.containerSize(); ++i) {
      params.push_back(nameAndParams.tupleGetValueAt(i).symbolGet());
    }

    checkUniqueParameters(parser, params);

    // Bind the name to the params and unevaluated body.
    innerAR().m_functions[functionName] =
      FnParamsAndBody{std::move(params), body};
  }

  else {
    parser.throwError(
      "First element must be a symbol or a tagged tuple.");
  }

  // Evaluation result of `Define` is null.
  return GDValue();
}


void GDValueFnApplyTransform::checkUniqueParameters(
  GDValueParser const &defnParser,
  std::vector<GDVSymbol> const &params)
{
  // Every parameter name must be distinct.
  std::set<GDVSymbol> names;
  GDVIndex i = 0;
  for (GDVSymbol const &param : params) {
    if (!setInsert(names, param)) {
      // Throw the error from a parser focused on the specific
      // offending parameter to get maximally precise location
      // information.
      defnParser.tupleGetValueAt(i).throwError(stringb(
        "Parameter name " << param.quotedString() <<
        " used more than once."));
    }
    ++i;
  }
}


// Push an element onto the front of a list in the ctor, and pop it off
// in the dtor.
//
// TODO: Move to `list-util`.
template <typename T>
class ListPushPopFront {
public:      // data
  // List being manipulated.
  std::list<T> &m_list;

public:       // methods
  explicit ListPushPopFront(std::list<T> &list, T &&t)
  :
    IMEMBFP(list)
  {
    m_list.push_front(std::move(t));
  }

  ~ListPushPopFront()
  {
    if (m_list.empty()) {
      // In a dtor, be defensive.  I don't think it's worth alerting
      // here (e.g, with `DEV_WARNING`) either; the list might have been
      // left in an unexpected state when an exception was thrown.
    }
    else {
      m_list.pop_front();
    }
  }
};


GDValue GDValueFnApplyTransform::handleFunctionApplication(
  GDValueParser const &appParser,
  GDVSymbol name,
  FnParamsAndBody const &paramsAndBody)
{
  EXN_CONTEXT(name.getSymbolName());

  // Check the number of arguments.
  GDVSize const numParams = paramsAndBody.m_params.size();
  appParser.checkContainerSize(numParams);

  // Build a new record where each parameter is bound to the evaluation
  // of each argument.  That evaluation does *not* see the new record.
  ActivationRecord newAR;
  for (GDVIndex i=0; i < numParams; ++i) {
    GDVSymbol param = paramsAndBody.m_params.at(i);
    GDValue const &arg = appParser.tupleGetValueAt(i).getValue();

    newAR.m_variables[param] = transform(arg);
  }

  // Evaluate the body in an environment where `newAR` is innermost.
  ListPushPopFront<ActivationRecord> pushPop(m_env, std::move(newAR));
  return transform(paramsAndBody.m_body);
}


GDValue GDValueFnApplyTransform::transform(GDValue const &v)
{
  GDValueParser parser(v);

  // Special form?
  if (parser.isTaggedTupleWTag("Define")) {
    return handleDefine(parser);
  }

  // Symbol with a definition?
  if (parser.isSymbol()) {
    if (auto valueOpt = lookupVariableName(parser.symbolGet())) {
      // Since symbol definitions are evaluated eagerly, at definition
      // time, we do not evaluate the value again at use time.
      return *valueOpt;
    }
  }

  // Function application?
  if (parser.isTaggedTuple()) {
    GDVSymbol name = parser.taggedContainerGetTag();

    // Is this the name of a defined function?
    if (auto paramsAndBodyOpt = lookupFunctionName(name)) {
      // Yes, call it.
      return handleFunctionApplication(
        parser, name, paramsAndBodyOpt->get());
    }
  }

  // Any case not already handled evaluates to itself, modulo evaluation
  // of contained elements.
  return GDValueTransform::transform(v);
}


std::size_t GDValueFnApplyTransform::environmentDepth() const
{
  return m_env.size();
}


std::set<GDVSymbol>
GDValueFnApplyTransform::innermostARVariableNames() const
{
  return innerARC().variableNames();
}


std::set<GDVSymbol>
GDValueFnApplyTransform::innermostARFunctionNames() const
{
  return innerARC().functionNames();
}


CLOSE_NAMESPACE(gdv)


// EOF
