# smbase/Makefile
# see license.txt for copyright and terms of use

# Main target.
all:
.PHONY: all


# ----------------------- Default configuration ------------------------
# The settings in this section provide defaults that can be overridden
# in config.mk or personal.mk.

# C preprocessor, compiler and linker.
CC = gcc

# C++ compiler.
CXX = g++

# To use Clang to create MSVCRT-based executables on Windows:
#CC = clang.exe --target=x86_64-w64-windows-gnu
#CXX = clang++.exe --target=x86_64-w64-windows-gnu

# Flags to control generation of debug info.
DEBUG_FLAGS = -g

# Flags to enable dependency generation of .d files.
GENDEPS_FLAGS = -MMD

# Flags to control optimization.
OPTIMIZATION_FLAGS = -O2

# Flags to control compiler warnings.  The default is no warnings since
# it's aimed at people just using the library rather than developing it.
WARNING_FLAGS =

# Normal developer flags, which stop on warnings:
#WARNING_FLAGS = -Wall -Werror

# More warnings, but also disabling some that I don't want to fix.
#WARNING_FLAGS = -Wall -Werror -Wextra -Wno-type-limits -Wno-cast-function-type

# Warning flags for C++ specifically.  These are added to
# $(WARINING_FLAGS) for C++ compilation.
CXX_WARNING_FLAGS =

# Flags for C standard to use.
C_STD_FLAGS   = -std=c99

# Flags for C++ standard to use.
#
# I've decided to start trying to use std::optional, which means I need
# C++17.
CXX_STD_FLAGS = -std=c++17

# -D flags to pass to preprocessor.
DEFINES =

# -I flags to pass to preprocessor.
INCLUDES =

# Allow writing `#include "smbase/..."` within this directory.
INCLUDES = -I..

# Preprocessing flags.
CPPFLAGS = $(INCLUDES) $(DEFINES)

# Flags for the C and C++ compiler and preprocessor.
#
# Note: $(GENDEPS_FLAGS) are not included because these flags are used
# for linking too, and if that used $(GENDEPS_FLAGS) then the .d files
# for .o files would be overwritten with info for .exe files.
CFLAGS   = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(C_STD_FLAGS) $(CPPFLAGS)
CXXFLAGS = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(CXX_WARNING_FLAGS) $(CXX_STD_FLAGS) $(CPPFLAGS)

# System libraries needed.
SYSLIBS =

# Math library for executables that use floating-point functions.  I
# haven't historically needed this, but now I do with GCC 9.3 on Linux?
MATHLIB = -lm

# Directory to put compilation outputs into (including executables and
# library archives, despite the name).  Part of the point of having this
# directory is to facilitate compiling with multiple compilers, with the
# results being kept separate.
OBJDIR = obj

# Main output file, the smbase library archive.
#
# TODO: Rename this variable.
THIS = $(OBJDIR)/libsmbase.a

# Libraries to link with when creating test executables.
LIBS = $(THIS) $(SYSLIBS)

# Flags to add to a link command *in addition* to either $(CFLAGS) or
# $(CXXFLAGS), depending on whether C++ modules are included.
LDFLAGS =

# Flags to add to C and C++ compilations to enable coverage measurement.
COVERAGE_CFLAGS = --coverage
COVERAGE_CXXFLAGS = --coverage

# When measuring coverage with gcov, if optimization is enabled then it
# falsely reports some lines as uncovered.
COVERAGE_OPTIMIZATION_FLAGS =

# Some other tools.
RM      = rm -f
AR      = ar
RANLIB  = ranlib
PYTHON3 = python3

# https://mypy-lang.org/
MYPY    = mypy

# How to invoke run-compare-expect.py.
RUN_COMPARE_EXPECT = $(PYTHON3) ./run-compare-expect.py

# Run whatever command follows with a timeout.  This is used for the
# unit tests, where it's not uncommon for me to have an infinite loop,
# which is slightly annoying to kill when the test is run from within
# my editor.
TIMEOUT_PROGRAM = timeout
TIMEOUT_VALUE = 5
RUN_WITH_TIMEOUT = $(TIMEOUT_PROGRAM) $(TIMEOUT_VALUE)

# This invokes a script called 'mygcov', which is a personal wrapper
# around 'gcov' that filters out some common false positives.  You
# could just replace this with 'gcov' if you don't have that script.
GCOV = mygcov

# Set to 1 to activate the rules that generate source code.
GENSRC = 0

# Set to 1 to compute code coverage.
COVERAGE = 0

# If 1, generate the *.o.json files used to create
# compile_commands.json.  This requires that we are using Clang.
CREATE_O_JSON_FILES = 0

# If 1, hook the `mypy` checks into the `check` target.
ENABLE_MYPY = 0

# Path to the null device.
#
# I made this with the intention of being able to use 'NUL' on Windows,
# but that does not really work because we end up invoking a mixture of
# cygwin and native executables.
DEV_NULL = /dev/null

# How to invoke include-what-you-use (include-what-you-use.org).
IWYU := include-what-you-use


# ------------------------- User customization -------------------------
# Allow customization of the above variables in a separate file.  Just
# create config.mk and/or personal.mk with desired settings.
#
# Common things to set during development:
#
#   WERROR = -Werror
#   WARNING_FLAGS = -Wall $(WERROR)
#   OPTIMIZATION_FLAGS =
#   CXX_WARNING_FLAGS = -Wsuggest-override
#
-include config.mk
-include personal.mk


# ---------------------- Configuration adjustment ----------------------
# React to user customizations.

# If coverage is enabled, turn on the relevant settings.
ifeq ($(COVERAGE),1)
  CFLAGS += $(COVERAGE_CFLAGS)
  CXXFLAGS += $(COVERAGE_CXXFLAGS)
  OPTIMIZATION_FLAGS = $(COVERAGE_OPTIMIZATION_FLAGS)
endif


# Check if the timeout program can be found.
ifneq ($(shell which $(TIMEOUT_PROGRAM) >$(DEV_NULL) 2>&1 && echo yes),yes)
  # Program not found so do not try to use it.
  RUN_WITH_TIMEOUT=
endif


# ------------------------------- Rules --------------------------------
# Standard stuff.
include sm-lib.mk


# Compile .cc to .o, also generating dependency files.
$(OBJDIR)/%.o: %.cc
	$(CREATE_OUTPUT_DIRECTORY)
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(call MJ_FLAG,$(OBJDIR)/$*) $(CXXFLAGS) $<

$(OBJDIR)/%.o: %.c
	$(CREATE_OUTPUT_DIRECTORY)
	$(CC) -c -o $@ $(GENDEPS_FLAGS) $(call MJ_FLAG,$(OBJDIR)/$*) $(CFLAGS) $<

# For occasional diagnostic purposes, a rule to preprocess explicitly.
%.ii: %.cc
	$(CXX) -E -o $@ $(CXXFLAGS) $<


# $(PRE_TARGET) is usually nothing, but can be set in personal.mk to
# direct 'make' to build something of interest first, in order to speed
# up reporting errors in a certain module.
pre-target: $(PRE_TARGET)
all: pre-target
.PHONY: pre-target


# -------------- Experimenting with m4 for related files ---------------
# This section is disabled by default because the generated files are
# checked in to the repo, but when git checks them out, they have random
# timestamps so 'make' usually thinks some are out of date.  If you
# change the M4 sources, build with GENSRC=1 at least once.

ifeq ($(GENSRC),1)

# Default target will refresh any out of date generated sources.
all: gensrc
gensrc: sobjlist.h objlist.h strobjdict.h strsobjdict.h strintdict.h

# Run M4 to generate a single file.  Parameters:
#   $(1): Name of target file.
#   $(2): Name of source file.
define RUN_M4

$(1): $(2)
	@# Forcibly remove target, which may be read-only.
	$(RM) $(1)
	@#
	@# Generate tmp.h first.  That way, if m4 fails, we will not
	@# have written an empty or partial target file.
	m4 -Dm4_output=$(1) --prefix-builtins $(2) > tmp.h
	@#
	@# Now move tmp.h into the target location and make the latter
	@# read-only so I don't accidentally edit it.
	mv -f tmp.h $(1)
	chmod a-w $(1)

endef # RUN_M4

# All of the M4 input files know what to do based on the name of the
# output file, so do not need any additional parameters.
$(eval $(call RUN_M4,sobjlist.h,xobjlist.h))
$(eval $(call RUN_M4,objlist.h,xobjlist.h))
$(eval $(call RUN_M4,strobjdict.h,xstrobjdict.h))
$(eval $(call RUN_M4,strsobjdict.h,xstrobjdict.h))
$(eval $(call RUN_M4,strintdict.h,xstrobjdict.h))

endif # GENSRC==1


# ---------------------------- Main target -----------------------------

# Library source files in "LANG=C sort" order
SRCS :=
SRCS += autofile.cc
SRCS += bdffont.cc
SRCS += bflatten.cc
SRCS += binary-stdin.cc
SRCS += bit2d.cc
SRCS += bitarray.cc
SRCS += boxprint.cc
SRCS += breaker.cc
SRCS += c-string-reader.cc
SRCS += codepoint.cc
SRCS += counting-ostream.cc
SRCS += crc.cc
SRCS += cycles.c
SRCS += d2vector.c
SRCS += datablok.cc
SRCS += datetime.cc
SRCS += dev-warning.cc
SRCS += exc.cc
SRCS += file-line-col.cc
SRCS += flatten.cc
SRCS += functional-set.cc
SRCS += gcc-options.cc
SRCS += gdvalue-reader.cc
SRCS += gdvalue-write-options.cc
SRCS += gdvalue-writer.cc
SRCS += gdvalue.cc
SRCS += gdvsymbol.cc
SRCS += gdvtuple.cc
SRCS += gprintf.c
SRCS += growbuf.cc
SRCS += hashline.cc
SRCS += hashtbl.cc
SRCS += indexed-string-table.cc
SRCS += missing.cc
SRCS += mypopen.c
SRCS += mysig.cc
SRCS += nonport.cc
SRCS += objcount.cc
SRCS += ofstreamts.cc
SRCS += parsestring.cc
SRCS += point.cc
SRCS += pprint.cc
SRCS += rack-allocator.cc
SRCS += reader.cc
SRCS += refct-serf.cc
SRCS += run-process.cc
SRCS += sm-compare.cc
SRCS += sm-env.cc
SRCS += sm-file-util.cc
SRCS += sm-integer.cc
SRCS += sm-rc-obj.cc
SRCS += sm-regex.cc
SRCS += sm-stristr.cc
SRCS += sm-test.cc
SRCS += sm-trace.cc
SRCS += sm-unixutil.c
SRCS += srcloc.cc
SRCS += str.cc
SRCS += strdict.cc
SRCS += strhash.cc
SRCS += string-hash.cc
SRCS += string-util.cc
SRCS += stringf.cc
SRCS += stringset.cc
SRCS += strtable.cc
SRCS += strtokp.cc
SRCS += strutil.cc
SRCS += svdict.cc
SRCS += syserr.cc
SRCS += temporary-file.cc
SRCS += trace.cc
SRCS += trdelete.cc
SRCS += tree-print.cc
SRCS += utf8-reader.cc
SRCS += utf8-writer.cc
SRCS += vdtllist.cc
SRCS += voidlist.cc
SRCS += vptrmap.cc
SRCS += warn.cc
SRCS += xarithmetic.cc
SRCS += xoverflow.cc

# Library object files.
OBJS := $(SRCS)
OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(OBJS))
OBJS := $(patsubst %.cc,$(OBJDIR)/%.o,$(OBJS))

# Pull in automatic dependencies created by $(GENDEPS_FLAGS).
-include $(OBJS:.o=.d)


$(THIS): $(OBJS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(RM) $(THIS)
	$(AR) -r $(THIS) $(OBJS)
	-$(RANLIB) $(THIS)

all: $(THIS)


# ---------------------------- Module tests ----------------------------
# Component test modules.
#
# The naming convention is "<module>-test" so that, in an alphabetic
# file name listing, the test is next to the module it tests.
UNIT_TEST_OBJS :=
UNIT_TEST_OBJS += array-test.o
UNIT_TEST_OBJS += array2d-test.o
UNIT_TEST_OBJS += arrayqueue-test.o
UNIT_TEST_OBJS += astlist-test.o
UNIT_TEST_OBJS += autofile-test.o
UNIT_TEST_OBJS += bdffont-test.o
UNIT_TEST_OBJS += bflatten-test.o
UNIT_TEST_OBJS += bit2d-test.o
UNIT_TEST_OBJS += bitarray-test.o
UNIT_TEST_OBJS += boxprint-test.o
UNIT_TEST_OBJS += c-string-reader-test.o
UNIT_TEST_OBJS += codepoint-test.o
UNIT_TEST_OBJS += counting-ostream-test.o
UNIT_TEST_OBJS += crc-test.o
UNIT_TEST_OBJS += cycles-test.o
UNIT_TEST_OBJS += d2vector-test.o
UNIT_TEST_OBJS += datablok-test.o
UNIT_TEST_OBJS += datetime-test.o
UNIT_TEST_OBJS += dict-test.o
UNIT_TEST_OBJS += distinct-number-test.o
UNIT_TEST_OBJS += dni-vector-test.o
UNIT_TEST_OBJS += exc-test.o
UNIT_TEST_OBJS += functional-set-test.o
UNIT_TEST_OBJS += gcc-options-test.o
UNIT_TEST_OBJS += gdvalue-test.o
UNIT_TEST_OBJS += gdvsymbol-test.o
UNIT_TEST_OBJS += gdvtuple-test.o
UNIT_TEST_OBJS += get-type-name-test.o
UNIT_TEST_OBJS += gprintf-test.o
UNIT_TEST_OBJS += growbuf-test.o
UNIT_TEST_OBJS += hashline-test.o
UNIT_TEST_OBJS += indexed-string-table-test.o
UNIT_TEST_OBJS += map-util-test.o
UNIT_TEST_OBJS += mypopen-test.o
UNIT_TEST_OBJS += mysig-test.o
UNIT_TEST_OBJS += nonport-test.o
UNIT_TEST_OBJS += objlist-test.o
UNIT_TEST_OBJS += objpool-test.o
UNIT_TEST_OBJS += optional-util-test.o
UNIT_TEST_OBJS += ordered-map-test.o
UNIT_TEST_OBJS += overflow-test.o
UNIT_TEST_OBJS += owner-test.o
UNIT_TEST_OBJS += parsestring-test.o
UNIT_TEST_OBJS += pprint-test.o
UNIT_TEST_OBJS += rack-allocator-test.o
UNIT_TEST_OBJS += reader-test.o
UNIT_TEST_OBJS += refct-serf-test.o
UNIT_TEST_OBJS += run-process-test.o
UNIT_TEST_OBJS += save-restore-test.o
UNIT_TEST_OBJS += set-util-test.o
UNIT_TEST_OBJS += sm-ap-int-test.o
UNIT_TEST_OBJS += sm-ap-uint-test.o
UNIT_TEST_OBJS += sm-env-test.o
UNIT_TEST_OBJS += sm-file-util-test.o
UNIT_TEST_OBJS += sm-integer-test.o
UNIT_TEST_OBJS += sm-is-equal-test.o
UNIT_TEST_OBJS += sm-pp-util-test.o
UNIT_TEST_OBJS += sm-rc-ptr-test.o
UNIT_TEST_OBJS += sm-regex-test.o
UNIT_TEST_OBJS += sm-stristr-test.o
UNIT_TEST_OBJS += sm-trace-test.o
UNIT_TEST_OBJS += sm-unique-ptr-test.o
UNIT_TEST_OBJS += sobjlist-test.o
UNIT_TEST_OBJS += srcloc-test.o
UNIT_TEST_OBJS += std-map-fwd-test.o
UNIT_TEST_OBJS += std-optional-fwd-test.o
UNIT_TEST_OBJS += std-set-fwd-test.o
UNIT_TEST_OBJS += std-string-fwd-test.o
UNIT_TEST_OBJS += std-string-view-fwd-test.o
UNIT_TEST_OBJS += std-variant-fwd-test.o
UNIT_TEST_OBJS += std-vector-fwd-test.o
UNIT_TEST_OBJS += str-test.o
UNIT_TEST_OBJS += strdict-test.o
UNIT_TEST_OBJS += strhash-test.o
UNIT_TEST_OBJS += string-hash-test.o
UNIT_TEST_OBJS += string-util-test.o
UNIT_TEST_OBJS += stringf-test.o
UNIT_TEST_OBJS += stringset-test.o
UNIT_TEST_OBJS += strutil-test.o
UNIT_TEST_OBJS += svdict-test.o
UNIT_TEST_OBJS += syserr-test.o
UNIT_TEST_OBJS += taillist-test.o
UNIT_TEST_OBJS += temporary-file-test.o
UNIT_TEST_OBJS += trdelete-test.o
UNIT_TEST_OBJS += tree-print-test.o
UNIT_TEST_OBJS += utf8-test.o
UNIT_TEST_OBJS += vdtllist-test.o
UNIT_TEST_OBJS += vector-push-pop-test.o
UNIT_TEST_OBJS += vector-util-test.o
UNIT_TEST_OBJS += voidlist-test.o
UNIT_TEST_OBJS += vptrmap-test.o
UNIT_TEST_OBJS += xassert-test.o

# Master unit test module.
UNIT_TEST_OBJS += unit-tests.o

# The unit test objects also go into $(OBJDIR).
UNIT_TEST_OBJS := $(patsubst %.o,$(OBJDIR)/%.o,$(UNIT_TEST_OBJS))

-include $(UNIT_TEST_OBJS:.o=.d)

$(OBJDIR)/unit-tests.exe: $(UNIT_TEST_OBJS) $(THIS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(UNIT_TEST_OBJS) $(LIBS)

all: $(OBJDIR)/unit-tests.exe


# Rule for tests that have dedicated .cc files, which is currently just
# binary-stdin-test.exe.
$(OBJDIR)/%-test.exe: $(OBJDIR)/%-test.o $(THIS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(CXX) -o $@ $(call MJ_FLAG,$(OBJDIR)/$*-test) $(CXXFLAGS) $(LDFLAGS) $< $(LIBS)


# Create a read-only file I can try to inspect in sm-file-util-test.cc.
test.dir/read-only.txt:
	$(CREATE_OUTPUT_DIRECTORY)
	echo "this file is read-only" >$@
	chmod a-w $@


# If we are missing an expect file, just make an empty one.
test/%.expect:
	$(CREATE_OUTPUT_DIRECTORY)
	touch $@

# This variable is used as a dependency on targets that I only want to
# run after the unit tests have passed, both because the unit tests are
# more fundamental, and because I do not want the output of both kinds
# of tests being interleaved during parallel `make`.  Those rules do not
# actually have a dependency on this file.
AFTER_UNIT_TESTS := out/unit-tests.exe.ok

# Run one unit test and compare to expected output.
out/%.unit.ok: test/%.expect $(AFTER_UNIT_TESTS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual out/$*.actual \
	  --expect test/$*.expect \
	  --path-not-found-replacer \
	  env VERBOSE=1 $(OBJDIR)/unit-tests.exe $*
	touch $@

check: out/boxprint.unit.ok
check: out/gdvalue.unit.ok
check: out/tree_print.unit.ok


# ------------------------- binary-stdin-test --------------------------
all: $(OBJDIR)/binary-stdin-test.exe

out/binary-stdin-test.ok: $(OBJDIR)/binary-stdin-test.exe
	$(CREATE_OUTPUT_DIRECTORY)
	@#
	@# Generate a file with all 256 bytes.
	$(OBJDIR)/binary-stdin-test.exe allbytes out/allbytes.bin
	@#
	@# Test reading stdin with 'read'.
	$(OBJDIR)/binary-stdin-test.exe read0 allbytes < out/allbytes.bin
	@#
	@# Test writing stdout with 'write'.
	$(OBJDIR)/binary-stdin-test.exe allbytes write1 > out/allbytes-actual.bin
	cmp out/allbytes-actual.bin out/allbytes.bin
	@#
	@# Test reading stdin with 'fread'.
	$(OBJDIR)/binary-stdin-test.exe fread_stdin allbytes < out/allbytes.bin
	@#
	@# Test writing stdout with 'fwrite'.
	$(OBJDIR)/binary-stdin-test.exe allbytes fwrite_stdout > out/allbytes-actual.bin
	cmp out/allbytes-actual.bin out/allbytes.bin
	@#
	@# Test reading stdin with 'cin.read'.
	$(OBJDIR)/binary-stdin-test.exe cin_read allbytes < out/allbytes.bin
	@#
	@# Test writing stdout with 'cout.write'
	$(OBJDIR)/binary-stdin-test.exe allbytes cout_write > out/allbytes-actual.bin
	cmp out/allbytes-actual.bin out/allbytes.bin
	@#
	@# Done.
	touch $@

check: out/binary-stdin-test.ok


# ---------------------------- call-abort ------------------------------
# Test program used by run-process-test.cc.
$(OBJDIR)/call-abort.exe: $(OBJDIR)/call-abort.o
	$(CREATE_OUTPUT_DIRECTORY)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $<


# ------------------------------- gdvn ---------------------------------
# Program to read and write GDVN.
#
# This rule creates a .o file rather that directly compiling the .cc
# file because if I do the latter with coverage enabled when the .gcno
# and .gcda files end up in the source directory instead of $(OBJDIR).
#
# TODO: Adjust LDFLAGS so that CXXFLAGS is not needed here when
# compiling with coverage enabled.
#
$(OBJDIR)/gdvn.exe: $(OBJDIR)/gdvn.o $(THIS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $< $(LIBS)

all: $(OBJDIR)/gdvn.exe


# Create an empty expected-output file if necessary.
test/gdvn/%-expect: test/gdvn/%
	$(CREATE_OUTPUT_DIRECTORY)
	touch $@

# Run a file through gdvn.exe.
out/gdvn/%-ok: test/gdvn/% test/gdvn/%-expect $(OBJDIR)/gdvn.exe $(AFTER_UNIT_TESTS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual out/gdvn/$*-actual \
	  --expect test/gdvn/$*-expect \
	  $(OBJDIR)/gdvn.exe $<
	touch $@

# Files with which to test gdvn.
GDVN_INPUTS :=
GDVN_INPUTS += $(wildcard test/gdvn/bad/*.gdvn)
GDVN_INPUTS += $(wildcard test/gdvn/*.gdvn)

# .ok files resulting from running the tests.
GDVN_OKFILES := $(patsubst test/gdvn/%,out/gdvn/%-ok,$(GDVN_INPUTS))


.PHONY: check-gdvn
check-gdvn: $(GDVN_OKFILES)


# Run one input through `gdvn` via stdin.
out/gdvn/123-stdin.ok: test/gdvn/123.gdvn test/gdvn/123-stdin-expect $(OBJDIR)/gdvn.exe $(AFTER_UNIT_TESTS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual out/gdvn/123-stdin-actual \
	  --expect test/gdvn/123-stdin-expect \
	  sh -c "$(OBJDIR)/gdvn.exe < $<"
	touch $@

check-gdvn: out/gdvn/123-stdin.ok


check: check-gdvn



# ---------------------- EXPECT_COMPILE_TEST_FAIL ----------------------
# As the recipe for `out/$1-test-error-$2.txt.ok`, attempt to compile
# `$1-test.cc` with `ERRNUM` set to $2 in order to enable a particular
# piece of syntax that should cause a compilation error.
#
# The generated rule has `$1-test.o` as a prerequisite in order to delay
# running it until after we have confirmed that the file compiles
# successfully without any seeded errors.
#
# Make the `ok` file a prerequisite of `check-$1-errs`, which then
# functions as a group name for all the tests with the same `$1`.
#
define EXPECT_COMPILE_TEST_FAIL

out/$1-test-error-$2.txt.ok: $(OBJDIR)/$1-test.o
	$$(CREATE_OUTPUT_DIRECTORY)
	if $$(CXX) -c -o out/$1-test-error-$2.o \
	          $$(CXXFLAGS) -DERRNUM=$2 $1-test.cc \
	          >out/$1-test-error-$2.txt 2>&1; then \
	  echo "$1-test compile error case $2 passed but should have failed!"; \
	  exit 2; \
	else \
	  echo "failed as expected: out/$1-test-error-$2.txt"; \
	fi
	touch $$@

.PHONY: check-$1-errs
check-$1-errs: out/$1-test-error-$2.txt.ok

endef # EXPECT_COMPILE_TEST_FAIL


# ------------------------- check-compile-errs -------------------------
# gdvalue-test errors
$(eval $(call EXPECT_COMPILE_TEST_FAIL,gdvalue,1))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,gdvalue,2))
check-compile-errs: check-gdvalue-errs


# distinct-number-test errors
$(eval $(call EXPECT_COMPILE_TEST_FAIL,distinct-number,1))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,distinct-number,2))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,distinct-number,3))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,distinct-number,4))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,distinct-number,5))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,distinct-number,6))
check-compile-errs: check-distinct-number-errs


# dni-vector-test errors
$(eval $(call EXPECT_COMPILE_TEST_FAIL,dni-vector,1))
$(eval $(call EXPECT_COMPILE_TEST_FAIL,dni-vector,2))
check-compile-errs: check-dni-vector-errs


# Check that things that should *not* compile in fact do not.
.PHONY: check-compile-errs
check: check-compile-errs


# -------------------------- run unit tests ----------------------------
out/unit-tests.exe.ok: $(OBJDIR)/unit-tests.exe $(OBJDIR)/call-abort.exe test.dir/read-only.txt
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_WITH_TIMEOUT) $(OBJDIR)/unit-tests.exe
	touch $@

check: out/unit-tests.exe.ok


# ------------------- test run-compare-expect.py -----------------------
test/rce_expect_%:
	$(CREATE_OUTPUT_DIRECTORY)
	touch $@

# Run a command through run-compare-expect.py.
out/rce_%.ok: test/rce_expect_% run-compare-expect.py
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual out/rce_actual_$* \
	  --expect test/rce_expect_$* \
	  $(RCE_CMD_$*)
	touch $@


# Test RCE itself, specifically the --drop-lines option.
#
# Explicitly set UPDATE_EXPECT=0 for this test to get consistent
# behavior even if the user has that variable set.
RCE_CMD_droplines := \
  env UPDATE_EXPECT=0 $(RUN_COMPARE_EXPECT) \
    --expect $(DEV_NULL) \
    --drop-lines 'extern' \
    --drop-lines '^x//' \
    --drop-lines '^\s*$$' \
    cat test/cycles.head.h

rce-tests: out/rce_droplines.ok


# Test hex digit replacement.
RCE_CMD_hexreplace := --hex-replacer cat test/hashex.txt

rce-tests: out/rce_hexreplace.ok


# Test no hex digit replacement.
RCE_CMD_nohexreplace := cat test/hashex.txt

rce-tests: out/rce_nohexreplace.ok


# Test separators (default) versus --no-separators.
RCE_CMD_separators0 := sh -c 'echo out; echo err >&2; exit 0'
RCE_CMD_separators3 := sh -c 'echo out; echo err >&2; exit 3'
RCE_CMD_noseparators0 := --no-separators sh -c 'echo out; echo err >&2; exit 0'
RCE_CMD_noseparators3 := --no-separators sh -c 'echo out; echo err >&2; exit 3'

rce-tests: out/rce_separators0.ok
rce-tests: out/rce_separators3.ok
rce-tests: out/rce_noseparators0.ok
rce-tests: out/rce_noseparators3.ok


# Test --no-stderr.
RCE_CMD_withstderr := sh -c 'echo out; echo err >&2'
RCE_CMD_nostderr := --no-stderr sh -c 'echo out; echo err >&2'
RCE_CMD_nostderr_nosep := --no-stderr --no-separators sh -c 'echo out; echo err >&2'

rce-tests: out/rce_withstderr.ok
rce-tests: out/rce_nostderr.ok
rce-tests: out/rce_nostderr_nosep.ok


# Test --chdir.
RCE_CMD_chdir1 := --chdir fonts ls sample1.bdf

rce-tests: out/rce_chdir1.ok


.PHONY: rce-tests
check: rce-tests


# ------------------- test create-tuple-class.py -----------------------
# Rewrite one header and implementation file.
out/test/ctc/in/%.ok: test/ctc/in/%.h test/ctc/in/%.cc create-tuple-class.py
	$(CREATE_OUTPUT_DIRECTORY)
	$(PYTHON3) ./create-tuple-class.py \
	  --prefix=out/ \
	  test/ctc/in/$*.h
	$(RUN_COMPARE_EXPECT) \
	  --expect test/ctc/exp/$*.h \
	  --no-separators --no-stderr \
	  cat out/test/ctc/in/$*.h
	$(RUN_COMPARE_EXPECT) \
	  --expect test/ctc/exp/$*.cc \
	  --no-separators --no-stderr \
	  cat out/test/ctc/in/$*.cc
	$(CXX) -c -o out/test/ctc/in/$*.o -I. out/test/ctc/in/$*.cc
	touch $@

.PHONY: check-ctc
check-ctc: out/test/ctc/in/foo.ok

check: check-ctc


# -------------- check create-tuple-class.py outputs -------------------
# Set of header files that use create-tuple-class.py.
CTC_HEADERS :=
CTC_HEADERS += xarithmetic.h
CTC_HEADERS += xoverflow.h

# Corresponding implementation files.
CTC_IMPL_FILES := $(CTC_HEADERS:.h=.cc)

# Check that all of the CTC-generated code is up to date.
out/ctc-up-to-date.ok: $(CTC_HEADERS) $(CTC_IMPL_FILES) create-tuple-class.py
	$(CREATE_OUTPUT_DIRECTORY)
	$(PYTHON3) ./create-tuple-class.py \
	  --check $(CTC_HEADERS)
	touch $@

check: out/ctc-up-to-date.ok


# ------------------------------- mypy ---------------------------------
# Run `mypy` on a script.
out/%.mypy.ok: %
	$(CREATE_OUTPUT_DIRECTORY)
	$(MYPY) --strict $*
	touch $@

.PHONY: check-mypy
check-mypy: out/create-tuple-class.py.mypy.ok
check-mypy: out/find-extra-deps.py.mypy.ok
check-mypy: out/get-file-descriptions.py.mypy.ok
check-mypy: out/run-compare-expect.py.mypy.ok

ifeq ($(ENABLE_MYPY),1)
check: check-mypy
endif


# ---------------------- `using namespace` check -----------------------
# Verify that `using namespace` does not appear in any header.
ALL_HEADERS := $(wildcard *.h)
out/no-using-namespace-in-header.ok: $(ALL_HEADERS)
	$(CREATE_OUTPUT_DIRECTORY)
	@if grep 'using namespace' $(ALL_HEADERS); then \
	  echo "Some headers have 'using namespace'."; \
	  exit 2; \
	else \
	  exit 0; \
	fi
	touch $@

.PHONY: check-ad-hoc
check-ad-hoc: out/no-using-namespace-in-header.ok

check: check-ad-hoc


# ----------------------------- coverage -------------------------------
# Run gcov to produce .gcov files.  This requires having compiled with
# COVERAGE=1.
.PHONY: run-gcov
run-gcov:
	$(GCOV) --object-directory $(OBJDIR) $(SRCS)

# Clean up previous gcov output.
.PHONY: gcov-clean
gcov-clean:
	$(RM) *.gcov $(OBJDIR)/*.gcda $(OBJDIR)/*.gcno


# ----------------------- compile_commands.json ------------------------
# Claim this is "phony" so we can regenerate with just "make
# compile_commands.json".  Also, I do not clean this file so I can make
# it using clang then switch back to gcc.
.PHONY: compile_commands.json

# This requires that the build was run with Clang and
# CREATE_O_JSON_FILES.
#
# The `sed` command removes the trailing comma from the last line only.
# IWYU does not tolerate them.
compile_commands.json:
	(echo "["; cat $(OBJDIR)/*.o.json | sed '$$ s/,$$//'; echo "]") > $@


# ---------------------------- index.html ------------------------------
HEADERS := $(wildcard *.h)
out/index.html.ok: index.html get-file-descriptions.py $(HEADERS)
	$(CREATE_OUTPUT_DIRECTORY)
	$(PYTHON3) ./get-file-descriptions.py \
	  --ignore='-fwd\.h$$' *.h
	touch $@

check: out/index.html.ok


# -------------------------------- IWYU --------------------------------
# Run IWYU on one file, but only after compilation has succeeded.
out/iwyu/%.cc.iwyu: %.cc $(OBJDIR)/%.o
	$(CREATE_OUTPUT_DIRECTORY)
	$(IWYU) $(CXXFLAGS) $< >$@ 2>&1
ifeq ($(PRINT_IWYU_OUT),1)
	cat $@
endif

out/iwyu/%.c.iwyu: %.c $(OBJDIR)/%.o
	$(CREATE_OUTPUT_DIRECTORY)
	$(IWYU) $(CFLAGS) $< >$@ 2>&1
ifeq ($(PRINT_IWYU_OUT),1)
	cat $@
endif

# Source files to apply IWYU to.
IWYU_SRCS :=
IWYU_SRCS += $(SRCS)

# Resulting per-file outputs.
IWYU_OUTPUTS := $(patsubst %,out/iwyu/%.iwyu,$(IWYU_SRCS))

# Run IWYU on all of $(IWYU_SRCS).
#
# For now, this target is only run manually.
iwyu.out: $(IWYU_OUTPUTS)
	cat $(IWYU_OUTPUTS) >$@


# ------------------------------- check --------------------------------
# The actual tests are all prerequisites of 'check' defined above.
.PHONY: check
check:
	@echo 'check: All tests passed.'

.PHONY: check-clean
check-clean:
	$(RM) -r out


# ------------------------------- clean --------------------------------
# delete compiling/editing byproducts
clean: gcov-clean check-clean
	$(RM) *~ gmon.out srcloc.tmp flattest.tmp
	$(RM) -r $(OBJDIR) test.dir

distclean: clean
	$(RM) compile_commands.json

# remove crap that vc makes
vc-clean:
	$(RM) *.plg *.[ip]db *.pch


# EOF
