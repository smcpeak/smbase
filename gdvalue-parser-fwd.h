// gdvalue-parser-fwd.h
// Forward decls for `gdvalue-parser.h`.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_GDVALUE_PARSER_FWD_H
#define SMBASE_GDVALUE_PARSER_FWD_H

namespace gdv {

class GDVNavStep;
class GDValueParser;
class XGDValueError;

template <typename T, typename Enable = void>
struct GDVPTo;

template <typename T, typename Enable = void>
struct GDVPToNew;

}

#endif // SMBASE_GDVALUE_PARSER_FWD_H
