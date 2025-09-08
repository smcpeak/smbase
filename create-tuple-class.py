#!/usr/bin/env python3
"""Create the methods of a "tuple" C++ class.

A tuple is a heterogenerous product of several element types.  It has
a primary constructor that accepts each of the element values, along
with a copy constructor, copy assignment operator, etc.  It is ordered
lexicographically by element value.

This script rewrites a section of a C++ header and implementation file
to contain the required method declarations and definitions.  It is
meant to be used on a file that is otherwise manually edited.

Within the class definition, put a line like:

  // ---- create-tuple-class: declarations for Foo +compare +write +move

In the implementation .cc file, put a line like (this one does not take
any options since they are specified in the line above):

  // ---- create-tuple-class: definitions for Foo

The declaration parser is quite simple, expecting field declarations to
be entirely contained on one line each.  If a field has an initializer,
it is used as the default argument for the primary constructor.

See test/ctc/in/foo.{h,cc} for example input and test/ctc/exp/foo.{h,cc}
for the corresponding example output.  See also ctc-test.{h,cc}.

Within a directive to generate declarations, the following options are
recognized:

  +move

    Generate move constructor and move assignment operator.

  +compare

    Generate relational comparison operators and a `compare` method.

  +write

    Generate `write`, `toString`, and `operator<<`.

  -writeDefn

    Omit the definition of `write` with "+write" or "+gdvWrite".

  +selfCheck

    Declare (but do not define) `selfCheck`, and emit calls to it in
    the ctors and assignment operators.

  +gdvWrite

    Generate `operator GDValue()`, and use it for `write`, `toString`
    and `operator<<`.  This flag is incompatible with "+write".

  +gdvRead

    Generate a ctor that accepts `GDValueParser`.
"""

import argparse              # argparse
import difflib               # difflib.unified_diff
import os                    # os.getenv
import re                    # re.compile
import signal                # signal.signal
import subprocess            # subprocess.run
import sys                   # sys.argv, sys.stderr
import time                  # time.sleep
import traceback             # traceback.print_exc

from boilerplate import *
from enum import Enum
from typing import Any, List, Optional


# If true, check that the generated code would be the same as what is
# already there.
checkMode: bool = False

# True if any differences were found.
checkModeDifferences: bool = False

# If true, just print the updated contents to stdout.
printMode: bool = False

# Prefix to apply to the written file name.  This can be used to
# redirect the output to a different place during testing.
outputPrefix: str = ""


# --------------------- General purpose utilities ----------------------
def readLinesNoNL(filename: str) -> list[str]:
  """Get the lines in 'filename' without newlines."""
  with open(filename) as file:
    return [line.rstrip("\n") for line in file.readlines()]


def writeLines(filename: str, lines: list[str]) -> None:
  """Write 'lines', each followed by a newline, to 'filename'."""
  with open(filename, "w") as file:
    for line in lines:
      print(line, file=file)


def writeUpdatedFile(fname: str, oldLines: list[str], newLines: list[str]) -> None:
  """Write `newLines` to `fname`, making a backup first."""

  debugPrint(f"writeUpdatedFile: fname={fname} "+
    f"oldLinesLen={len(oldLines)} newLinesLen={len(newLines)}")

  if oldLines == newLines:
    print(f"{fname} has up-to-date generated code.")
    return

  if checkMode:
    print(f"Generated contents are different for {fname}.")
    global checkModeDifferences
    checkModeDifferences = True

  elif printMode:
    print(f"---- {fname} ----")
    for line in newLines:
      print(line)

  else:
    fname = outputPrefix + fname
    writeLines(f"{fname}.bak", oldLines)
    writeLines(fname, newLines)
    print(f"Wrote updated contents for {fname}.")


# ------------------------- ClassOptions class -------------------------
class ClassOptions:
  """
  Represent the options associated with a particular class whose method
  declarations are to be generated.
  """

  def __init__(self) -> None:
    # All options default to False
    self.compare:     bool = False
    self.write:       bool = False
    self.noWriteDefn: bool = False
    self.move:        bool = False
    self.selfCheck:   bool = False
    self.gdvWrite:    bool = False
    self.gdvRead:     bool = False

  @classmethod
  def parse(cls, optionsString: str) -> "ClassOptions":
    """Parse an options string into an ClassOptions object we can use
    during code generation."""

    opts = cls()

    for opt in optionsString.split():
      if opt == "+compare":
        opts.compare = True

      elif opt == "+write":
        opts.write = True

      elif opt == "-writeDefn":
        opts.noWriteDefn = True

      elif opt == "+move":
        opts.move = True

      elif opt == "+selfCheck":
        opts.selfCheck = True

      elif opt == "+gdvWrite":
        opts.gdvWrite = True

      elif opt == "+gdvRead":
        opts.gdvRead = True

      else:
        die(f"Unrecognized option: {opt}")

    if opts.write and opts.gdvWrite:
      die('The "+write" and "+gdvWrite" options are incompatible.')

    return opts


# ------------------------------ Parsing -------------------------------
# This is quite crude but may suffice for now.
primitiveTypesRE = re.compile(r"\b(int|float|unsigned|bool)\b")

def isPrimitiveType(type: str) -> bool:
  """True if `type` is a C++ primitive type."""
  return bool(primitiveTypesRE.match(type))


identifierLetterRE = re.compile("^[a-zA-Z0-9_]$")

def isIdentifierLetter(c: str) -> bool:
  """True if `c` is a single-letter string where the letter could be
  part of a C++ identifier."""

  return bool(identifierLetterRE.match(c))


class Field:
  """Type, name, and optional default value."""
  def __init__(self, type: str, name: str, init: Optional[str]) -> None:
    # The elements partition the original declaration string.
    self.type = type
    self.name = name
    self.init = init

  def __eq__(self, other: Any) -> bool:
    if not isinstance(other, Field):
      return NotImplemented
    return (self.type == other.type and
            self.name == other.name and
            self.init == other.init)


def parseTypeAndName(typeAndName: str) -> Field:
  """Separate `typeAndName` into `type`, `name`, and optional
  `defaultValue` components."""

  # Index one beyond the last character that could be in the name.
  end_of_name = len(typeAndName)

  # Initializer?
  init: Optional[str] = None
  equals_index = typeAndName.rfind("=")
  if equals_index >= 0:
    # Include any spaces before "=" in the initializer.
    while equals_index > 0 and typeAndName[equals_index-1] == " ":
      equals_index -= 1
    init = typeAndName[equals_index:]
    end_of_name = equals_index

  # Collect the variable name by working backward from the end to find
  # the first character that is not part of an identifier.
  i = end_of_name - 1
  while i >= 0 and isIdentifierLetter(typeAndName[i]):
    i -= 1

  if not (0 < i and i < len(typeAndName)-1):
    die(f"Could not parse type and name: {typeAndName}")

  # Make `i` the index of the first character in the name.
  i += 1

  return Field(typeAndName[0:i], typeAndName[i:end_of_name], init)


def test_parseTypeAndName() -> None:
  """Unit tests for `parseTypeAndName`."""

  def one(typeAndName: str, expect: Field) -> None:
    actual = parseTypeAndName(typeAndName)
    assert(actual == expect)

  one("int x",
      Field("int ", "x", None))

  one("std::string str",
      Field("std::string ", "str", None))

  one("std::optional<int> m_intOpt = {}",
      Field("std::optional<int> ", "m_intOpt", " = {}"))

  one("std::optional<bool> m_wantDiagnostics = true",
      Field("std::optional<bool> ", "m_wantDiagnostics", " = true"))


# Generated line.
autoPrefixedLineRE = re.compile(r"^ */\*AUTO_CTC\*/")


# ------------------------- Header generation --------------------------
def generatePrimaryCtorParamName(fieldName: str) -> str:
  """Generate the name to use as the primary ctor parameter
  corresponding to `fieldName`."""

  # If the name begins with "m_", remove that.
  if fieldName.startswith("m_"):
    return fieldName[2:]

  # Otherwise, append an underscore.
  else:
    return fieldName + "_"


# By "primary", I mean a ctor that takes a parameter list similar to the
# set of data members, as opposed to the default (no-arg) ctor or the
# copy or move ctors (which take a reference to the containing class).
class CtorType(Enum):
  """Which ctor we are generating."""
  PRIMARY_COPY = 1      # Primary ctor that copies parameters.
  PRIMARY_MOVE = 2      # Primary ctor that moves parameters.


class WantDefaults(Enum):
  """Whether to emit default values."""
  TRUE = 1              # Emit defaults.
  FALSE = 2             # Do not emit defaults.


def generatePrimaryCtorParam(
  field: Field,
  ct: CtorType,
  wantDefaults: WantDefaults) -> str:

  """Generate the declaration of a primary constructor parameter
  corresponding to field `name` of a tuple class.  If `move`, this is
  the move constructor.  If `withDefaults`, include default values when
  present."""

  # Pass non-primitives by `const` reference.
  type: str = field.type
  if not isPrimitiveType(type):
    if ct == CtorType.PRIMARY_MOVE:
      type = f"{type}&&"
    else:
      type = f"{type}const &"

  # Remove leading "m_".
  name: str = generatePrimaryCtorParamName(field.name)

  init: str = ""
  if wantDefaults == WantDefaults.TRUE and field.init:
    init = field.init

  return f"{type}{name}{init}";


def hasNonPrimitiveField(fields: list[Field]) -> bool:
  """True if any element of `fields` is not a primitive type."""

  for field in fields:
    if not isPrimitiveType(field.type):
      return True

  # All fields were primitive.
  return False


def generatePrimaryCtorParams(fields: list[Field], ct: CtorType) -> str:
  """Generate a string that contains the parameter declarations for the
  primary constructor of a tuple class containing `fields`."""

  return ", ".join(
    [generatePrimaryCtorParam(field, ct, WantDefaults.TRUE)
     for field
     in fields])


def addAutoPrefix(line: str) -> str:
  """Put the /*AUTO_CTC*/ prefix in front of `line`."""

  ret: str = "/*AUTO_CTC*/"

  # Only add a space if `line` is not empty.
  if line != "":
    ret += " " + line

  return ret;


def generateDeclarations(
  curClass: str,
  curIndentation: str,
  fields: list[Field],
  options: ClassOptions) -> list[str]:

  """Generate and return the declarations for methods of `curClass`,
  which has overall indentation `curIndentation`, subject to
  `options`."""

  # Use a variable to specify `noexcept` in case I want to make this an
  # option later.
  noexcept: str = " noexcept"

  out: list[str] = []

  # explicit Foo(int x, float y, std::string const &z);
  out.append(f"explicit {curClass}(" +
    f"{generatePrimaryCtorParams(fields, CtorType.PRIMARY_COPY)});")

  enableMoveOps: bool = options.move

  if enableMoveOps and hasNonPrimitiveField(fields):
    # The primary move constructor takes all class-typed arguments by
    # rvalue reference.  This is not ideal since a call site might have
    # a mix of moveable and non-moveable arguments.  However, the usual
    # solution is to use a perfectly-forwarding template constructor,
    # which is a large jump in complexity to solve a minor problem.
    #
    # explicit Foo(int x, float y, std::string &&z);
    out.append(f"explicit {curClass}(" +
      f"{generatePrimaryCtorParams(fields, CtorType.PRIMARY_MOVE)});")

  # Foo(Foo const &obj) noexcept;
  out.append(f"{curClass}({curClass} const &obj){noexcept};")

  if enableMoveOps:
    # Foo(Foo &&obj) noexcept;
    out.append(f"{curClass}({curClass} &&obj){noexcept};")

  if options.selfCheck:
    # void selfCheck() const;
    out.append("void selfCheck() const;")

  # Foo &operator=(Foo const &obj) noexcept;
  out.append(f"{curClass} &operator=({curClass} const &obj){noexcept};")

  if enableMoveOps:
    # Foo &operator=(Foo &&obj) noexcept;
    out.append(f"{curClass} &operator=({curClass} &&obj){noexcept};")

  if options.compare:
    # // For +compare:
    out.append("// For +compare:")

    # friend int compare(Foo const &a, Foo const &b);
    out.append(f"friend int compare({curClass} const &a, {curClass} const &b);")

    # DEFINE_FRIEND_RELATIONAL_OPERATORS(Foo)
    out.append(f"DEFINE_FRIEND_RELATIONAL_OPERATORS({curClass})")

  def common_write_methods() -> None:
    # std::string toString() const;
    out.append("std::string toString() const;")

    # void write(std::ostream &os) const;
    out.append("void write(std::ostream &os) const;")

    # friend std::ostream &operator<<(std::ostream &os, Foo const &obj);
    out.append(f"friend std::ostream &operator<<(std::ostream &os, {curClass} const &obj);")

  if options.write:
    # // For +write:
    out.append("// For +write:")

    common_write_methods()

  if options.gdvWrite:
    # // For +gdvWrite:
    out.append("// For +gdvWrite:")

    # operator gdv::GDValue() const;
    out.append("operator gdv::GDValue() const;")

    common_write_methods()

  if options.gdvRead:
    # // For +gdvRead:
    out.append("// For +gdvRead:")

    # explicit Foo(gdv::GDValueParser const &p);
    out.append(f"explicit {curClass}(gdv::GDValueParser const &p);")

  # Prepend prefixes.
  out = [curIndentation + "  " + addAutoPrefix(line) for line in out]

  return out


def processHeader(headerFname: str) -> None:
  """Read `headerFname` and scan it for directives to generate code.
  Also scan the corresponding implementation file."""

  # Original file contents.
  origHeaderLines = readLinesNoNL(headerFname)

  debugPrint(f"{headerFname} has {len(origHeaderLines)} lines")

  # New file contents.
  newHeaderLines = []

  # Match a `class` or `struct` declaration first line.
  classDeclFirstLineRE = re.compile(r"""
    ^(\s*)                             # 1: indentation
    (?:class|struct)\s+                # `class` or `struct` keyword
    (\S+)                              # 2: class name

    (?:
      \s+final                         # optional `final`
    )?

    (?:                                # optional superclass specifier
      \s*:\s*                            # colon with optional surrounding whitespace
      (?:
        (?:public|private|protected)\s*    # optional access specifier
      )
      (\S+)                              # 3: superclass name
    )?

    \s*                                # optional trailing whitespace
    \{                                 # opening brace
    """, re.VERBOSE)

  # Match a line that indicates where to insert code and its options.
  beginLineRE = re.compile(r".*create-tuple-class: declarations for (\S+)(.*)")

  # Match a line containing a field declaration.
  fieldDeclLineRE = re.compile(r"^ +([a-zA-Z_][^();]+);$")

  # RE to exclude things that otherwise look like fields.
  nonFieldRE = re.compile(r"\busing\b")

  # Match the last line.
  classDeclLastLineRE = re.compile(r"^( *)\};")

  # Current class indentation and name.
  curClass: Optional[str] = None
  curIndentation: Optional[str] = None
  curFields: list[Field] = []

  # Map from class name to its fields and options.
  classToSuperclass: dict[str, str] = {}
  classToFields: dict[str, list[Field]] = {}
  classToClassOptions: dict[str, ClassOptions] = {}

  # Compute the new contents.
  for i, line in enumerate(origHeaderLines):
    try:
      if bool(autoPrefixedLineRE.match(line)):
        # Delete this line.
        pass
      else:
        # Keep this line.
        newHeaderLines.append(line)

      if m := classDeclFirstLineRE.match(line):
        curIndentation = m.group(1)
        curClass = m.group(2)
        curSuperclass = m.group(3)
        curFields = []
        debugPrint(f"{i+1}: class decl: {curClass}")

        if curSuperclass is not None:
          classToSuperclass[curClass] = curSuperclass

      if m := fieldDeclLineRE.match(line):
        if not nonFieldRE.search(line):
          typeAndName = m.group(1)
          debugPrint(f"{i+1}: field: {typeAndName}")
          curFields.append(parseTypeAndName(typeAndName))

      if m := beginLineRE.match(line):
        directiveClass = m.group(1)
        if directiveClass != curClass:
          die(f"Found directive to write declarations for {directiveClass} "+
              f"but the current class is {curClass}.")
        options = ClassOptions.parse(m.group(2))
        debugPrint(f"{i+1}: begin decls: {options}")

        if curClass is None or curIndentation is None:
          die("no current class")
        else:
          classToFields[curClass] = curFields
          classToClassOptions[curClass] = options

          newHeaderLines += generateDeclarations(
            curClass,
            curIndentation,
            curFields,
            options)

          # Do not make further changes to this object once we have
          # added it to the map.
          #
          # TODO: I've made a mess of when these fields get reset.  I
          # should try to encapsulate them in a class.
          curFields = []

      if m := classDeclLastLineRE.match(line):
        debugPrint(f"{i+1}: end class")
        if m.group(1) == curIndentation:
          curClass = None
          curIndentation = None
          curFields = []

    except Exception as e:
      die(f"{headerFname}:{i+1}: {exceptionMessage(e)}")

  writeUpdatedFile(headerFname, origHeaderLines, newHeaderLines)

  processImplementationFile(
    headerFname,
    classToSuperclass,
    classToFields,
    classToClassOptions)


# ------------------- Implementation file generation -------------------
def generatePrimaryCtorParamsSeparateLines(
  fields: list[Field],
  ct: CtorType) -> list[str]:

  """Generate a list that contains the parameter declarations for the
  primary constructor of a tuple class containing `fields`, where each
  parameter will go to its own line."""

  out = []

  for i, field in enumerate(fields):
    terminator = "," if i+1 < len(fields) else ")"

    out.append("  " +
      generatePrimaryCtorParam(field, ct, WantDefaults.FALSE) +
      terminator)

  return out


def generatePrimaryCtorInit(fieldName: str) -> str:
  """Generate the primary ctor initializer for `fieldName`."""

  paramName = generatePrimaryCtorParamName(fieldName)

  if fieldName.startswith("m_"):
    # Use the purpose-built macro.
    return f"IMEMBFP({paramName})"
  else:
    return f"{fieldName}({paramName})"


def generatePrimaryMoveCtorInit(fieldName: str) -> str:
  """Generate the primary move ctor initializer for `fieldName`."""

  paramName = generatePrimaryCtorParamName(fieldName)

  if fieldName.startswith("m_"):
    # Use the purpose-built macro.
    return f"IMEMBMFP({paramName})"
  else:
    return f"{fieldName}(std::move({paramName}))"


def generateCtorInits(
  superclass: Optional[str],
  fields: list[Field],
  kind: str) -> list[str]:

  """Generate the lines that initialize a constructor.  The ctor kind is
  indicated by `kind`, which is either "primary", "primaryMove", or the
  name of a macro to invoke for each member."""

  out: list[str] = []

  if superclass is not None:
    # The call to the base class is generated by pretending we have a
    # field with the base class's name.
    fields = [Field("<dontcare>", superclass, None)] + fields

  for i, field in enumerate(fields):
    if field.name == superclass:
      if kind == "primary":
        init = f"{superclass}()"
      elif kind == "MDMEMB":
        init = f"{superclass}(std::move(obj))"
      else:
        init = f"{superclass}(obj)"

    elif kind == "primary":
      init = generatePrimaryCtorInit(field.name)

    elif kind == "primaryMove":
      init = generatePrimaryMoveCtorInit(field.name)

    else:
      init = f"{kind}({field.name})"

    comma = "," if i+1 != len(fields) else ""
    leadIn = ": " if i == 0 else "  "

    out.append(f"  {leadIn}{init}{comma}")

  return out


def generateCallsPerField(fields: list[Field], func: str) -> list[str]:
  """Generate lines that call `func` for each field in `fields`."""

  out: list[str] = []

  for field in fields:
    out.append(f"  {func}({field.name});")

  return out


def generateCtorBody(options: ClassOptions) -> list[str]:
  """Generate and return the code for the body of a ctor."""

  out: list[str] = []

  if options.selfCheck:
    out.append("{")
    out.append("  selfCheck();")
    out.append("}")
  else:
    out.append("{}")

  return out


def maybeGenerateSelfCheck(
  options: ClassOptions,
  indent: str) -> list[str]:
  """If +selfCheck, return `indent` + "selfCheck();"""

  if options.selfCheck:
    return [indent + "selfCheck();"]
  else:
    return []


def generateDefinitions(
  curClass: str,
  superclass: Optional[str],
  fields: list[Field],
  options: ClassOptions) -> list[str]:

  """Generate and return the definitions for methods of `curClass`,
  subject to `options`."""

  noexcept: str = " noexcept"

  out: list[str] = []

  # Foo::Foo(
  #   int x,
  #   float y,
  #   std::string const &z)
  #   : IMEMBFP(x),          // insert "Super()" if superclass
  #     IMEMBFP(y),
  #     IMEMBFP(z)
  # {
  #   selfCheck();           // If +selfCheck.
  # }
  out.append(f"{curClass}::{curClass}(")
  out.extend(generatePrimaryCtorParamsSeparateLines(fields, CtorType.PRIMARY_COPY))
  out.extend(generateCtorInits(superclass, fields, "primary"))
  out.extend(generateCtorBody(options))
  out.append("")

  enableMoveOps: bool = options.move

  if enableMoveOps and hasNonPrimitiveField(fields):
    # Foo::Foo(
    #   int x,
    #   float y,
    #   std::string &&z)
    #   : IMEMBMFP(x),       // insert "Super()" if superclass
    #     IMEMBMFP(y),
    #     IMEMBMFP(z)
    # {
    #   selfCheck();         // If +selfCheck.
    # }
    out.append(f"{curClass}::{curClass}(")
    out.extend(generatePrimaryCtorParamsSeparateLines(fields, CtorType.PRIMARY_MOVE))
    out.extend(generateCtorInits(superclass, fields, "primaryMove"))
    out.extend(generateCtorBody(options))
    out.append("")

  # Foo::Foo(Foo const &obj) noexcept
  #   : DMEMB(m_x),          // insert "Super(obj)" if superclass
  #     DMEMB(m_y),
  #     DMEMB(m_z)
  # {
  #   selfCheck();           // If +selfCheck.
  # }
  out.append(f"{curClass}::{curClass}({curClass} const &obj){noexcept}")
  out.extend(generateCtorInits(superclass, fields, "DMEMB"))
  out.extend(generateCtorBody(options))
  out.append("")

  if enableMoveOps:
    # Foo::Foo(Foo &&obj) noexcept
    #   : MDMEMB(m_x),       // insert "Super(std::move(obj))" if superclass
    #     MDMEMB(m_y),
    #     MDMEMB(m_z)
    # {
    #   selfCheck();         // If +selfCheck.
    # }
    out.append(f"{curClass}::{curClass}({curClass} &&obj){noexcept}")
    out.extend(generateCtorInits(superclass, fields, "MDMEMB"))
    out.extend(generateCtorBody(options))
    out.append("")

  # Foo &Foo::operator=(Foo const &obj) noexcept
  # {
  #   if (this != &obj) {
  #     Super::operator(obj);                    // if superclass
  #     CMEMB(x);
  #     CMEMB(y);
  #     CMEMB(z);
  #     selfCheck();         // If +selfCheck.
  #   }
  #   return *this;
  # }
  out.append(  f"{curClass} &{curClass}::operator=({curClass} const &obj){noexcept}")
  out.append(   "{")
  out.append(   "  if (this != &obj) {")
  if superclass is not None:
    out.append(f"    {superclass}::operator=(obj);")
  out.extend(        generateCallsPerField(fields, "  CMEMB"))
  out.extend(        maybeGenerateSelfCheck(options, "    "))
  out.append(   "  }")
  out.append(   "  return *this;")
  out.append(   "}")
  out.append(   "")

  if enableMoveOps:
    # Foo &Foo::operator=(Foo &&obj) noexcept
    # {
    #   if (this != &obj) {
    #     Super::operator=(std::move(obj));        // if superclass
    #     MCMEMB(x);
    #     MCMEMB(y);
    #     MCMEMB(z);
    #   }
    #   return *this;
    # }
    out.append(  f"{curClass} &{curClass}::operator=({curClass} &&obj){noexcept}")
    out.append(   "{")
    out.append(   "  if (this != &obj) {")
    if superclass is not None:
      out.append(f"    {superclass}::operator=(std::move(obj));")
    out.extend(        generateCallsPerField(fields, "  MCMEMB"))
    out.extend(        maybeGenerateSelfCheck(options, "    "))
    out.append(   "  }")
    out.append(   "  return *this;")
    out.append(   "}")
    out.append(   "")

  if options.compare:
    # int compare(Foo const &a, Foo const &b)
    # {
    #   RET_IF_COMPARE_MEMBERS(x);
    #   RET_IF_COMPARE_MEMBERS(y);
    #   RET_IF_COMPARE_MEMBERS(z);
    #   return 0;
    # }
    out += [
      f"int compare({curClass} const &a, {curClass} const &b)",
      "{"
    ] + (
          # My current objective is to derive from XBase, which does not
          # have a comparison operator, so just skip this...
          #([f"  RET_IF_COMPARE_SUBOBJS({superclass});"]
          #   if superclass is not None else []) +

          generateCallsPerField(fields, "RET_IF_COMPARE_MEMBERS")
        ) + [
      "  return 0;",
      "}",
      ""
    ]

  def emit_toString() -> List[str]:
    # std::string Foo::toString() const
    # {
    #   std::ostringstream oss;
    #   write(oss);
    #   return oss.str();
    # }
    return [
      f"std::string {curClass}::toString() const",
      "{",
      "  std::ostringstream oss;",
      "  write(oss);",
      "  return oss.str();",
      "}",
      ""
    ]

  def emit_operator_ll() -> List[str]:
    # std::ostream &operator<<(std::ostream &os, Foo const &obj)
    # {
    #   obj.write(os);
    #   return os;
    # }
    return [
      f"std::ostream &operator<<(std::ostream &os, {curClass} const &obj)",
      "{",
      "  obj.write(os);",
      "  return os;",
      "}",
      ""
    ]

  if options.write:
    out += emit_toString()

    if not options.noWriteDefn:
      # void Foo::write(std::ostream &os) const
      # {
      #   os << "{";
      #   WRITE_MEMBER(m_x);
      #   WRITE_MEMBER(m_y);
      #   WRITE_MEMBER(m_z);
      #   os << " }";
      # }
      out += [
        f"void {curClass}::write(std::ostream &os) const",
        "{",
        "  os << \"{\";"
      ] + generateCallsPerField(fields, "WRITE_MEMBER") + [
        "  os << \" }\";",
        "}",
        ""
      ]

    out += emit_operator_ll()

  if options.gdvWrite:
    out += emit_toString()
    out += emit_operator_ll()

    # Foo::operator gdv::GDValue() const
    # {
    #    using namespace gdv;
    #    GDValue m(GDVK_TAGGED_ORDERED_MAP, "Foo"_sym);
    #    GDV_WRITE_MEMBER_SYM(m_x);
    #    GDV_WRITE_MEMBER_SYM(m_y);
    #    GDV_WRITE_MEMBER_SYM(m_z);
    #    return m;
    # }
    out += [
      f"{curClass}::operator gdv::GDValue() const",
       "{",
       "  using namespace gdv;",
      f"  GDValue m(GDVK_TAGGED_ORDERED_MAP, \"{curClass}\"_sym);"
    ] + generateCallsPerField(fields, "GDV_WRITE_MEMBER_SYM") + [
       "  return m;",
       "}",
       ""
    ]

    if not options.noWriteDefn:
      # void Foo::write(std::ostream &os) const
      # {
      #   operator gdv::GDValue().writeIndented(os);
      # }
      out += [
        f"void {curClass}::write(std::ostream &os) const",
         "{",
         "  operator gdv::GDValue().writeIndented(os);",
         "}",
         ""
      ]

  if options.gdvRead:
    # Foo::Foo(gdv::GDValueParser const &p)
    #   : GDVP_READ_MEMBER_SYM(m_x),
    #     GDVP_READ_MEMBER_SYM(m_y),
    #     GDVP_READ_MEMBER_SYM(m_z)
    # {
    #    p.checkTaggedOrderedMapTag("Foo");
    # }
    out.append(f"{curClass}::{curClass}(gdv::GDValueParser const &p)")
    out.extend(generateCtorInits(
      superclass, fields, "GDVP_READ_MEMBER_SYM"))
    out += [
       "{",
      f"  p.checkTaggedOrderedMapTag(\"{curClass}\");",
       "}",
       ""
    ]

  # Prepend prefixes.
  out = [addAutoPrefix(line) for line in out]

  return out


def processImplementationFile(
  headerFname: str,
  classToSuperclass: dict[str, str],
  classToFields: dict[str, list[Field]],
  classToClassOptions: dict[str, ClassOptions]) -> None:
  """Determine the implementation file for `headerFname` and process it."""

  assert(headerFname.endswith(".h"))
  implFname = headerFname[:-2]+".cc"
  debugPrint(f"implFname: {implFname}")

  if not os.path.isfile(implFname):
    die(f"Expected to find implementation file at \"{implFname}\" "+
        f"based on header file name \"{headerFname}\" but it is "+
        f"not there.")

  # Original file lines.
  origImplLines = readLinesNoNL(implFname)

  # New file lines.
  newImplLines = []

  # Match a line that indicates where to insert code.
  beginLineRE = re.compile(r".*create-tuple-class: definitions for (\S+)")

  for i, line in enumerate(origImplLines):
    try:
      if bool(autoPrefixedLineRE.match(line)):
        # Delete this line.
        pass
      else:
        # Keep this line.
        newImplLines.append(line)

      if m := beginLineRE.match(line):
        curClass = m.group(1)
        debugPrint(f"{i+1}: begin, curClass={curClass}")

        if curClass not in classToClassOptions:
          die(f"Found directive to create definitions for {curClass}, "+
              f"but its declarations were not seen.")

        superclass: Optional[str] = classToSuperclass.get(curClass, None)
        fields: list[Field] = classToFields[curClass]
        options: ClassOptions = classToClassOptions[curClass]

        newImplLines += generateDefinitions(
          curClass,
          superclass,
          fields,
          options)

    except Exception as e:
      die(f"{implFname}:{i+1}: {exceptionMessage(e)}")

  writeUpdatedFile(implFname, origImplLines, newImplLines)


# -------------------------------- main --------------------------------
def main() -> None:
  # Run unit tests first.
  test_parseTypeAndName()

  # Parse command line.
  parser = argparse.ArgumentParser()
  parser.add_argument("--check", action="store_true",
    help="Check if the generated code is up to date; do not change anything.")
  parser.add_argument("--print", action="store_true",
    help="Print the generated code instead of updating the file.")
  parser.add_argument("--prefix",
    help="Prefix to prepend to output file names.")
  parser.add_argument("headerFiles", nargs='+',
    help="Names of headers containing directives to process.")
  opts = parser.parse_args()

  global checkMode
  global printMode
  global outputPrefix

  if opts.check:
    checkMode = True
  if opts.print:
    printMode = True
  if opts.prefix:
    outputPrefix = opts.prefix

  for fname in opts.headerFiles:
    if not fname.endswith(".h"):
      die(f"Header file name must end with \".h\": {fname!r}.")

  for fname in opts.headerFiles:
    processHeader(fname)

  if checkModeDifferences:
    print(f"Re-run {sys.argv[0]} without --check to re-generate them.")
    sys.exit(1)


if __name__ == "__main__":
  call_main(main)


# EOF
