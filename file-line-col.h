// file-line-col.h
// FileLineCol, a data triple.
//
// Whereas srcloc.h is concerned with a compact representation, this
// class's main goal is ease of use.

#ifndef SMBASE_FILE_LINE_COL_H
#define SMBASE_FILE_LINE_COL_H

#include <cstddef>                     // std::size_t
#include <optional>                    // std::optional
#include <string>                      // std::string


// A line and column number.
class LineCol {
public:      // data
  // 1-based line number of the location where the error occurred.
  int m_line;

  // 1-based column number of the error location.
  //
  // A 0 value can be used to represent the character before the first
  // on a line in situations where the previous line's length is
  // unavailable.
  //
  // Currently, the way this class is used by Reader, it actually tracks
  // a *byte* count from the line start rather than a character count.
  int m_column;

  // Byte offset from the start of the data.
  std::size_t m_byteOffset;

public:      // methods
  LineCol(int line, int column, std::size_t byteOffset) noexcept;

  LineCol(LineCol const &obj) = default;
  LineCol& operator=(LineCol const &obj) = default;

  // If 'c' is '\n' then increment the line and reset the column to 1.
  // Otherwise, increment the column.
  void incrementForChar(int c);

  // Decrement the column number unless it is already zero.  Does not
  // change the line number.
  void decrementColumn();

  // Try to undo the effect of 'incrementForChar(c)'.  This would be
  // used along with something like 'std::istream::putback(c)'.
  void decrementForChar(int c);
};


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

  // Manipulate the line/col.
  void incrementForChar(int c)         { m_lc.incrementForChar(c); }
  void decrementColumn()               { m_lc.decrementColumn(); }
  void decrementForChar(int c)         { m_lc.decrementForChar(c); }

  // Extract the line/col.
  LineCol const &getLineCol() const { return m_lc; }

  // Set the line/col.
  void setLineCol(LineCol const &lc) { m_lc = lc; }
};


#endif // SMBASE_FILE_LINE_COL_H
