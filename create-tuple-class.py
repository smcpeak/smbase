#!/usr/bin/env python3
"""Create the methods of a "tuple" C++ class.

A tuple is a heterogenerous product of several element types.  It has
a primary constructor that accepts each of the element values, along
with a copy constructor, copy assignment operator, etc.  It is ordered
lexicographically by element value.

This script rewrites a section of a C++ header and implementation file
to contain the required method declarations and definitions.  It is
meant to be used on a file that is otherwise manually edited.

See test/ctc/in/foo.{h,cc} for example input and test/ctc/exp/foo.{h,cc}
for the corresponding example output.

Within a directive to generate declarations, the following options are
recognized:

  +move

    Generate move constructor and move assignment operator.

  +compare

    Generate relational comparison operators and a `compare` method.

  +write

    Generate `write`, `toString`, and `operator<<`.

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

from typing import Optional


# -------------- BEGIN: boilerplate -------------
# These are things I add at the start of every Python program to
# allow better error reporting.

# Positive if debug is enabled, with higher values enabling more printing.
debugLevel = 0
if debugEnvVal := os.getenv("DEBUG"):
  debugLevel = int(debugEnvVal)

def debugPrint(str: str) -> None:
  """Debug printout when DEBUG >= 2."""
  if debugLevel >= 2:
    print(str)

# Ctrl-C: interrupt the interpreter instead of raising an exception.
signal.signal(signal.SIGINT, signal.SIG_DFL)

class Error(Exception):
  """A condition to be treated as an error."""
  pass

def die(message: str) -> None:
  """Throw a fatal Error with message."""
  raise Error(message)

def exceptionMessage(e: BaseException) -> str:
  """Turn exception 'e' into a human-readable message."""
  t = type(e).__name__
  s = str(e)
  if s:
    return f"{t}: {s}"
  else:
    return f"{t}"

def call_main() -> None:
  """Call main() and catch exceptions."""
  try:
    main()

  except SystemExit as e:
    raise      # Let this one go, otherwise sys.exit gets "caught".

  except BaseException as e:
    print(f"{exceptionMessage(e)}", file=sys.stderr)
    if (debugLevel >= 1):
      traceback.print_exc(file=sys.stderr)
    sys.exit(2)
# --------------- END: boilerplate --------------


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


# This is quite crude but may suffice for now.
primitiveTypesRE = re.compile(r"\b(int|float|unsigned|bool)\b")

def isPrimitiveType(type: str) -> bool:
  """True if `type` is a C++ primitive type."""
  return bool(primitiveTypesRE.match(type))


def generatePrimaryCtorParamName(fieldName: str) -> str:
  """Generate the name to use as the primary ctor parameter
  corresponding to `fieldName`."""

  # If the name begins with "m_", remove that.
  if fieldName.startswith("m_"):
    return fieldName[2:]

  # Otherwise, append an underscore.
  else:
    return fieldName + "_"


def generatePrimaryCtorParam(type: str, name: str) -> str:
  """Generate the declaration of a primary constructor parameter
  corresponding to field `name` of a tuple class."""

  # Pass non-primitives by `const` reference.
  if not isPrimitiveType(type):
    type = f"{type}const &"

  # Remove leading "m_".
  name = generatePrimaryCtorParamName(name)

  return f"{type}{name}";


# A field is a tuple of its type and name.
Field = tuple[str, str]


def generatePrimaryCtorParams(fields: list[Field]) -> str:
  """Generate a string that contains the parameter declarations for the
  primary constructor of a tuple class containing `fields`."""

  return ", ".join(
    [generatePrimaryCtorParam(type, name) for (type, name) in fields])


# The options are expressed as a map from option key to bool.  This is
# populated by `parseOptions`.
OptionsMap = dict[str, bool]


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
  options: OptionsMap) -> list[str]:

  """Generate and return the declarations for methods of `curClass`,
  which has overall indentation `curIndentation`, subject to
  `options`."""

  # Use a variable to specify `noexcept` in case I want to make this an
  # option later.
  noexcept: str = " noexcept"

  out: list[str] = []

  # explicit Foo(int x, float y, std::string const &z);
  out.append(f"explicit {curClass}({generatePrimaryCtorParams(fields)});")

  # Foo(Foo const &obj) noexcept;
  out.append(f"{curClass}({curClass} const &obj){noexcept};")

  enableMoveOps: bool = options.get("move", False)

  if enableMoveOps:
    # Foo(Foo &&obj) noexcept;
    out.append(f"{curClass}({curClass} &&obj){noexcept};")

  # Foo &operator=(Foo const &obj) noexcept;
  out.append(f"{curClass} &operator=({curClass} const &obj){noexcept};")

  if enableMoveOps:
    # Foo &operator=(Foo &&obj) noexcept;
    out.append(f"{curClass} &operator=({curClass} &&obj){noexcept};")

  if options.get("compare", False):
    # // For +compare:
    out.append("// For +compare:")

    # friend int compare(Foo const &a, Foo const &b);
    out.append(f"friend int compare({curClass} const &a, {curClass} const &b);")

    # DEFINE_FRIEND_RELATIONAL_OPERATORS(Foo)
    out.append(f"DEFINE_FRIEND_RELATIONAL_OPERATORS({curClass})")

  if options.get("write", False):
    # // For +write:
    out.append("// For +write:")

    # std::string toString() const;
    out.append("std::string toString() const;")

    # void write(std::ostream &os) const;
    out.append("void write(std::ostream &os) const;")

    # friend std::ostream &operator<<(std::ostream &os, Foo const &obj);
    out.append(f"friend std::ostream &operator<<(std::ostream &os, {curClass} const &obj);")

  # Prepend prefixes.
  out = [curIndentation + "  " + addAutoPrefix(line) for line in out]

  return out


def parseOptions(optionsString: str) -> OptionsMap:
  """Parse an options string into a map we can use during code
  generation."""

  ret: OptionsMap = {}

  for opt in optionsString.split():
    if opt == "+compare":
      ret["compare"] = True

    elif opt == "+write":
      ret["write"] = True

    elif opt == "+move":
      ret["move"] = True

    else:
      die(f"Unrecognized option: {opt}")

  return ret


identifierLetterRE = re.compile("^[a-zA-Z0-9_]$")

def isIdentifierLetter(c: str) -> bool:
  """True if `c` is a single-letter string where the letter could be
  part of a C++ identifier."""

  return bool(identifierLetterRE.match(c))


def parseTypeAndName(typeAndName: str) -> Field:
  """Separate `typeAndName` into `type` and `name` components."""

  # Go backward from the end to find the first character that is not
  # part of an identifier.
  i = len(typeAndName) - 1
  while i >= 0 and isIdentifierLetter(typeAndName[i]):
    i -= 1

  if not (0 < i and i < len(typeAndName)-1):
    die(f"Could not parse type and name: {typeAndName}")

  # Make `i` the index of the first character in the name.
  i += 1

  return (typeAndName[0:i], typeAndName[i:])


# Generated line.
autoPrefixedLineRE = re.compile(r"^ */\*AUTO_CTC\*/")


def processHeader(headerFname: str) -> None:
  """Read `headerFname` and scan it for directives to generate code.
  Also scan the corresponding implementation file."""

  # Original file contents.
  origHeaderLines = readLinesNoNL(headerFname)

  debugPrint(f"{headerFname} has {len(origHeaderLines)} lines")

  # New file contents.
  newHeaderLines = []

  # Match a `class` or `struct` declaration first line.
  classDeclFirstLineRE = re.compile(
    r"^( *)(?:class|struct) +(\S+)" +
    #  ^ indent              ^ class name
    r"(?:\s*:\s*(?:(?:public|private|protected)\s*)(\S+))?\s*\{")
    #                                              ^ superclass name

  # Match a line that indicates where to insert code and its options.
  beginLineRE = re.compile(r".*create-tuple-class: declarations for (\S+)(.*)")

  # Match a line containing a field declaration.
  fieldDeclLineRE = re.compile(r"^ +([a-zA-Z_][^();]+);$")

  # Match the last line.
  classDeclLastLineRE = re.compile(r"^( *)\};")

  # Current class indentation and name.
  curClass: Optional[str] = None
  curIndentation: Optional[str] = None
  curFields: list[Field] = []

  # Map from class name to its fields and options.
  classToSuperclass: dict[str, str] = {}
  classToFields: dict[str, list[Field]] = {}
  classToOptions: dict[str, OptionsMap] = {}

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
        typeAndName = m.group(1)
        debugPrint(f"{i+1}: field: {typeAndName}")
        curFields.append(parseTypeAndName(typeAndName))

      if m := beginLineRE.match(line):
        directiveClass = m.group(1)
        if directiveClass != curClass:
          die(f"Found directive to write declarations for {directiveClass} "+
              f"but the current class is {curClass}.")
        options = parseOptions(m.group(2))
        debugPrint(f"{i+1}: begin decls: {options}")

        if curClass is None or curIndentation is None:
          die("no current class")
        else:
          classToFields[curClass] = curFields
          classToOptions[curClass] = options

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
    classToOptions)


def generatePrimaryCtorParamsSeparateLines(fields: list[Field]) -> list[str]:
  """Generate a list that contains the parameter declarations for the
  primary constructor of a tuple class containing `fields`, where each
  parameter will go to its own line."""

  out = []

  for i, (type, name) in enumerate(fields):
    terminator = "," if i+1 < len(fields) else ")"

    out.append("  " + generatePrimaryCtorParam(type, name) + terminator)

  return out


def generatePrimaryCtorInit(fieldName: str) -> str:
  """Generate the primary ctor initializer for `fieldName`."""

  paramName = generatePrimaryCtorParamName(fieldName)
  return f"{fieldName}({paramName})"


def generateCtorInits(
  superclass: Optional[str],
  fields: list[Field],
  kind: str) -> list[str]:

  """Generate the lines that initialize a constructor.  The ctor kind
  is indicated by `kind`, which is either "primary" or the name of a
  macro to invoke for each member."""

  out: list[str] = []

  if superclass is not None:
    fields = [("<dontcare>", superclass)] + fields

  for i, field in enumerate(fields):
    fieldName = field[1]

    if fieldName == superclass:
      if kind == "primary":
        init = f"{superclass}()"
      elif kind == "MDMEMB":
        init = f"{superclass}(std::move(obj))"
      else:
        init = f"{superclass}(obj)"

    elif kind == "primary":
      init = generatePrimaryCtorInit(fieldName)

    else:
      init = f"{kind}({fieldName})"

    comma = "," if i+1 != len(fields) else ""
    leadIn = ": " if i == 0 else "  "

    out.append(f"  {leadIn}{init}{comma}")

  return out


def generateCallsPerField(fields: list[Field], func: str) -> list[str]:
  """Generate lines that call `func` for each field in `fields`."""

  out: list[str] = []

  for field in fields:
    fieldName = field[1]

    out.append(f"  {func}({fieldName});")

  return out


def generateDefinitions(
  curClass: str,
  superclass: Optional[str],
  fields: list[Field],
  options: OptionsMap) -> list[str]:

  """Generate and return the definitions for methods of `curClass`,
  subject to `options`."""

  noexcept: str = " noexcept"

  out: list[str] = []

  # Foo::Foo(
  #   int x,
  #   float y,
  #   std::string const &z)
  #   : m_x(x),              // insert "Super()" if superclass
  #     m_y(y),
  #     m_z(z)
  # {}
  out += [
    f"{curClass}::{curClass}("
  ] + (
         generatePrimaryCtorParamsSeparateLines(fields) +
         generateCtorInits(superclass, fields, "primary")
      ) + [
    "{}",
    ""
  ]

  # Foo::Foo(Foo const &obj) noexcept
  #   : DMEMB(m_x),          // insert "Super(obj)" if superclass
  #     DMEMB(m_y),
  #     DMEMB(m_z)
  # {}
  out += [
    f"{curClass}::{curClass}({curClass} const &obj){noexcept}",
  ] + generateCtorInits(superclass, fields, "DMEMB") + [
    "{}",
    ""
  ]

  enableMoveOps: bool = options.get("move", False)

  if enableMoveOps:
    # Foo::Foo(Foo &&obj) noexcept
    #   : MDMEMB(m_x),         // insert "Super(std::move(obj))" if superclass
    #     MDMEMB(m_y),
    #     MDMEMB(m_z)
    # {}
    out += [
      f"{curClass}::{curClass}({curClass} &&obj){noexcept}"
    ] + generateCtorInits(superclass, fields, "MDMEMB") + [
      "{}",
      ""
    ]

  # Foo &Foo::operator=(Foo const &obj) noexcept
  # {
  #   if (this != &obj) {
  #     Super::operator(obj);                    // if superclass
  #     CMEMB(x);
  #     CMEMB(y);
  #     CMEMB(z);
  #   }
  #   return *this;
  # }
  out += [
    f"{curClass} &{curClass}::operator=({curClass} const &obj){noexcept}",
    "{",
    "  if (this != &obj) {"
  ] + (
        ([f"    {superclass}::operator=(obj);"]
           if superclass is not None else []) +
        generateCallsPerField(fields, "  CMEMB")
      ) + [
    "  }",
    "  return *this;",
    "}",
    ""
  ]

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
    out += [
      f"{curClass} &{curClass}::operator=({curClass} &&obj){noexcept}",
      "{",
      "  if (this != &obj) {"
    ] + (
          ([f"    {superclass}::operator=(std::move(obj));"]
             if superclass is not None else []) +
          generateCallsPerField(fields, "  MCMEMB")
        ) + [
      "  }",
      "  return *this;",
      "}",
      ""
    ]

  if options.get("compare", False):
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

  if options.get("write", False):
    # std::string Foo::toString() const
    # {
    #   std::ostringstream oss;
    #   write(oss);
    #   return oss.str();
    # }
    out += [
      f"std::string {curClass}::toString() const",
      "{",
      "  std::ostringstream oss;",
      "  write(oss);",
      "  return oss.str();",
      "}",
      ""
    ]

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

    # std::ostream &operator<<(std::ostream &os, Foo const &obj)
    # {
    #   obj.write(os);
    #   return os;
    # }
    out += [
      f"std::ostream &operator<<(std::ostream &os, {curClass} const &obj)",
      "{",
      "  obj.write(os);",
      "  return os;",
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
  classToOptions: dict[str, OptionsMap]) -> None:
  """Determine the implementation file for `headerFname` and process it."""

  if not headerFname.endswith(".h"):
    die(f"Header file name must end with \".h\".")

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

        if curClass not in classToOptions:
          die(f"Found directive to create definitions for {curClass}, "+
              f"but its declarations were not seen.")

        superclass: Optional[str] = classToSuperclass.get(curClass, None)
        fields: list[Field] = classToFields[curClass]
        options: OptionsMap = classToOptions[curClass]

        newImplLines += generateDefinitions(
          curClass,
          superclass,
          fields,
          options)

    except Exception as e:
      die(f"{implFname}:{i+1}: {exceptionMessage(e)}")

  writeUpdatedFile(implFname, origImplLines, newImplLines)


def main() -> None:
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
    processHeader(fname)

  if checkModeDifferences:
    print(f"Re-run {sys.argv[0]} without --check to re-generate them.")
    sys.exit(1)

call_main()

# EOF
