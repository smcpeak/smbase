// file-line-col.h
// `FileLineCol`, a file/line/col data triple.
//
// Whereas srcloc.h is concerned with a compact representation, this
// class's main goal is ease of use.

#ifndef SMBASE_FILE_LINE_COL_H
#define SMBASE_FILE_LINE_COL_H

#include "file-line-col-fwd.h"         // fwds for this module

#include "smbase/compare-util-iface.h" // DECLARE_COMPARETO_AND_DEFINE_RELATIONALS
#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [n]
#include "smbase/line-col.h"           // smbase::LineCol
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

#include <cstddef>                     // std::size_t
#include <iosfwd>                      // std::ostream [n]
#include <optional>                    // std::optional
#include <string>                      // std::string


OPEN_NAMESPACE(smbase)


// A location in a file or stream that may or may not have a name.
class FileLineCol {
public:      // data
  // If the location is in a file with a known name, this is its name.
  //
  // Typically, this should either be an absolute path or a path
  // relative to the current directory, but the precise interpretation
  // is somewhat dependent on the user of this class.
  std::optional<std::string> m_fileName;

  // Line and column.
  LineCol m_lc;

public:      // methods
  // This is not marked 'explicit' because the conversion from optional
  // string to FileLineCol preserves the information.
  FileLineCol(std::optional<std::string> fileName = std::nullopt,
              int line = 1,
              int column = 1,
              std::size_t byteOffset = 0) noexcept;
  ~FileLineCol();

  FileLineCol(FileLineCol const &obj) = default;
  FileLineCol &operator=(FileLineCol const &obj) = default;

  // Assert invariants.
  void selfCheck() const;

  // Lexicographic comparison: file, lc
  DECLARE_COMPARETO_AND_DEFINE_RELATIONALS(FileLineCol);

  // If `m_fileName.has_value()`, write as "<file>: <line>:<col>".
  // Otherwise, write as "<line>:<col>".
  void write(std::ostream &os) const;
  friend std::ostream &operator<<(std::ostream &os, FileLineCol const &obj)
    { obj.write(os); return os; }

  // Return what `write` writes.
  std::string asString() const;

  // Returns a tagged ordered map of fields.
  operator gdv::GDValue() const;

  // Manipulate the line/col.
  void incrementForChar(int c)         { m_lc.incrementForChar(c); }
  void decrementColumn()               { m_lc.decrementColumn(); }
  void decrementForChar(int c)         { m_lc.decrementForChar(c); }

  // Extract the line/col.
  LineCol const &getLineCol() const { return m_lc; }

  // Set the line/col.
  void setLineCol(LineCol const &lc) { m_lc = lc; }
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_FILE_LINE_COL_H
