// pp-file-line.h
// `PreprocFileLine`, a "preprocessor" file and line.

// See license.txt for copyright and terms of use.

#ifndef SMBASE_PP_FILE_LINE_H
#define SMBASE_PP_FILE_LINE_H

#include "pp-file-line-fwd.h"          // fwds for this module

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE, IMEMBFP

#include <iosfwd>                      // std::ostream [n]

OPEN_NAMESPACE(smbase)


// A "preprocessor" file and line.  I say "preprocessor" because the
// expectation is these values are derived from the preprocessor
// symbols `__FILE__` and `__LINE__`, so I use that term to distinguish
// this class from others that also store file and line information.
class PreprocFileLine {
public:      // data
  // File name as obtained by `__FILE__`.  We assume the target string
  // does not get deallocated or changed.
  //
  // This has directory info if `__FILE__` does.
  char const *m_file;

  // Line number as obtained by `__LINE__`.
  int m_line;

public:
  PreprocFileLine(char const *file, int line)
    : IMEMBFP(file),
      IMEMBFP(line)
  {}

  char const *file() const
    { return m_file; }
  int line() const
    { return m_line; }

  // The portion of `m_file` that excludes directory info.
  char const *fileWithoutDir() const;

  // Write as "<file>:<line>" without directory info.
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(std::ostream &os, PreprocFileLine const &obj)
    { obj.write(os); return os; }
};


// Yield a `PreprocFileLine` for the place where this macro is
// expanded.
#define HERE_PREPROC_FILE_LINE \
  (smbase::PreprocFileLine(__FILE__, __LINE__))


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_PP_FILE_LINE_H
