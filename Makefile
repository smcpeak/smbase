# smbase/Makefile
# see license.txt for copyright and terms of use

# Main target.
THIS := libsmbase.a
all: pre-target $(THIS)
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

# Flags for C or C++ standard to use.
C_STD_FLAGS   = -std=c99
CXX_STD_FLAGS = -std=c++11

# -D flags to pass to preprocessor.
DEFINES =

# -I flags to pass to preprocessor.
INCLUDES =

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
AR      = ar
RANLIB  = ranlib
PYTHON3 = python3

# How to invoke run-compare-expect.py.
RUN_COMPARE_EXPECT = $(PYTHON3) ./run-compare-expect.py

# This invokes a script called 'mygcov', which is a personal wrapper
# around 'gcov' that filters out some common false positives.  You
# could just replace this with 'gcov' if you don't have that script.
GCOV = mygcov

# Set to 1 if we are cross-compiling, meaning the executables we make
# do not run on the build machine.
CROSS_COMPILE = 0

# Set to 1 to activate the rules that generate source code.
GENSRC = 0

# Set to 1 to compute code coverage.
COVERAGE = 0

# If 1, generate the *.o.json files used to create
# compile_commands.json.  This requires that we are using Clang.
CREATE_O_JSON_FILES = 0

# Path to the null device.
DEV_NULL = /dev/null


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


# ------------------------------- Rules --------------------------------
# Standard stuff.
include sm-lib.mk


# Compile .cc to .o, also generating dependency files.
%.o: %.cc
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(call MJ_FLAG,$*) $(CXXFLAGS) $<

%.o: %.cpp
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(call MJ_FLAG,$*) $(CXXFLAGS) $<

%.o: %.c
	$(CC) -c -o $@ $(GENDEPS_FLAGS) $(call MJ_FLAG,$*) $(CFLAGS) $<


# $(PRE_TARGET) is usually nothing, but can be set in personal.mk to
# direct 'make' to build something of interest first, in order to speed
# up reporting errors in a certain module.
pre-target: $(PRE_TARGET)
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
	rm -f $(1)
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
SRCS += breaker.cpp
SRCS += codepoint.cc
SRCS += crc.cpp
SRCS += cycles.c
SRCS += d2vector.c
SRCS += datablok.cc
SRCS += datetime.cc
SRCS += dev-warning.cc
SRCS += exc.cpp
SRCS += flatten.cc
SRCS += functional-set.cc
SRCS += gcc-options.cc
SRCS += gdvalue-write-options.cc
SRCS += gdvalue-writer.cc
SRCS += gdvalue.cc
SRCS += gdvsymbol.cc
SRCS += gprintf.c
SRCS += growbuf.cc
SRCS += hashline.cc
SRCS += hashtbl.cc
SRCS += missing.cpp
SRCS += mypopen.c
SRCS += mysig.cc
SRCS += nonport.cpp
SRCS += objcount.cc
SRCS += ofstreamts.cc
SRCS += parsestring.cc
SRCS += point.cc
SRCS += pprint.cc
SRCS += refct-serf.cc
SRCS += run-process.cc
SRCS += sm-compare.cc
SRCS += sm-file-util.cc
SRCS += sm-rc-obj.cc
SRCS += sm-to-std-string.cc
SRCS += sm-unixutil.c
SRCS += smregexp.cc
SRCS += srcloc.cc
SRCS += str.cc
SRCS += strdict.cc
SRCS += strhash.cc
SRCS += string-utils.cc
SRCS += stringset.cc
SRCS += strtable.cc
SRCS += strtokp.cpp
SRCS += strutil.cc
SRCS += svdict.cc
SRCS += syserr.cpp
SRCS += trace.cc
SRCS += trdelete.cc
SRCS += tree-print.cc
SRCS += vdtllist.cc
SRCS += voidlist.cc
SRCS += vptrmap.cc
SRCS += warn.cpp

# Library object files.
OBJS := $(SRCS)
OBJS := $(patsubst %.c,%.o,$(OBJS))
OBJS := $(patsubst %.cc,%.o,$(OBJS))
OBJS := $(patsubst %.cpp,%.o,$(OBJS))

# Pull in automatic dependencies created by $(GENDEPS_FLAGS).
-include $(OBJS:.o=.d)

$(THIS): $(OBJS)
	rm -f $(THIS)
	$(AR) -r $(THIS) $(OBJS)
	-$(RANLIB) $(THIS)


# ---------------------------- Module tests ----------------------------
# Test program targets.
#
# TODO: I would like to eliminate these stand-alone test programs in
# favor of testing as much as possible from unit-tests.exe.
TESTS :=
TESTS += bdffont.exe
TESTS += bitarray.exe
TESTS += boxprint.exe
TESTS += crc.exe
TESTS += cycles.exe
TESTS += d2vector.exe
TESTS += gprintf.exe
TESTS += hashline.exe
TESTS += nonport.exe
TESTS += pprint.exe
TESTS += srcloc.exe
TESTS += tarray2d.exe
TESTS += tarrayqueue.exe
TESTS += test-codepoint.exe
TESTS += test-datetime.exe
TESTS += test-owner.exe
TESTS += test-refct-serf.exe
TESTS += test-run-process.exe
TESTS += test-sm-file-util.exe
TESTS += test-stringset.exe
TESTS += test-tree-print.exe
TESTS += testarray.exe
TESTS += unit-tests.exe
TESTS += vptrmap.exe

tests: $(TESTS)


# this one is explicitly *not* linked against $(THIS)
nonport.exe: nonport.cpp nonport.h gprintf.o
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_NONPORT $(LDFLAGS) nonport.cpp gprintf.o $(SYSLIBS)

cycles.exe: cycles.h cycles.c
	$(CC) -o $@ $(CFLAGS) -DTEST_CYCLES $(LDFLAGS) cycles.c $(SYSLIBS)

crc.exe: crc.cpp
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_CRC $(LDFLAGS) crc.cpp $(SYSLIBS)

srcloc.exe: srcloc.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_SRCLOC $(LDFLAGS) srcloc.cc $(LIBS)

hashline.exe: hashline.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_HASHLINE $(LDFLAGS) hashline.cc $(LIBS)

gprintf.exe: gprintf.c gprintf.h
	$(CC) -o $@ $(CFLAGS) -DTEST_GPRINTF $(LDFLAGS) gprintf.c $(SYSLIBS)

vptrmap.exe: vptrmap.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_VPTRMAP $(LDFLAGS) vptrmap.cc $(LIBS)

pprint.exe: pprint.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_PPRINT $(LDFLAGS) pprint.cc $(LIBS)

boxprint.exe: boxprint.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BOXPRINT $(LDFLAGS) boxprint.cc $(LIBS)

tarrayqueue.exe: tarrayqueue.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) tarrayqueue.cc $(LIBS)

testarray.exe: testarray.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) testarray.cc $(LIBS)

bitarray.exe: bitarray.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BITARRAY $(LDFLAGS) bitarray.cc $(LIBS)

d2vector.exe: d2vector.c $(THIS)
	$(CC) -o $@ $(CFLAGS) -DTEST_D2VECTOR $(LDFLAGS) d2vector.c $(LIBS) $(MATHLIB)

bdffont.exe: bdffont.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BDFFONT $(LDFLAGS) bdffont.cc $(LIBS)

tarray2d.exe: tarray2d.cc array2d.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_TARRAY2D $(LDFLAGS) tarray2d.cc $(LIBS)

# Component test modules.
#
# The naming convention is "<module>-test" so that, in an alphabetic
# file name listing, the test is next to the module it tests.
UNIT_TEST_OBJS :=
UNIT_TEST_OBJS += array-test.o
UNIT_TEST_OBJS += astlist-test.o
UNIT_TEST_OBJS += autofile-test.o
UNIT_TEST_OBJS += bflatten-test.o
UNIT_TEST_OBJS += bit2d-test.o
UNIT_TEST_OBJS += counting-ostream-test.o
UNIT_TEST_OBJS += datablok-test.o
UNIT_TEST_OBJS += dict-test.o
UNIT_TEST_OBJS += functional-set-test.o
UNIT_TEST_OBJS += gcc-options-test.o
UNIT_TEST_OBJS += gdvalue-test.o
UNIT_TEST_OBJS += growbuf-test.o
UNIT_TEST_OBJS += map-utils-test.o
UNIT_TEST_OBJS += mypopen-test.o
UNIT_TEST_OBJS += mysig-test.o
UNIT_TEST_OBJS += objlist-test.o
UNIT_TEST_OBJS += objpool-test.o
UNIT_TEST_OBJS += overflow-test.o
UNIT_TEST_OBJS += parsestring-test.o
UNIT_TEST_OBJS += sm-pp-util-test.o
UNIT_TEST_OBJS += sm-rc-ptr-test.o
UNIT_TEST_OBJS += smregexp-test.o
UNIT_TEST_OBJS += sobjlist-test.o
UNIT_TEST_OBJS += str-test.o
UNIT_TEST_OBJS += strdict-test.o
UNIT_TEST_OBJS += strhash-test.o
UNIT_TEST_OBJS += string-utils-test.o
UNIT_TEST_OBJS += strutil-test.o
UNIT_TEST_OBJS += svdict-test.o
UNIT_TEST_OBJS += taillist-test.o
UNIT_TEST_OBJS += trdelete-test.o
UNIT_TEST_OBJS += vdtllist-test.o
UNIT_TEST_OBJS += vector-utils-test.o
UNIT_TEST_OBJS += voidlist-test.o

# Master unit test module.
UNIT_TEST_OBJS += unit-tests.o

-include $(UNIT_TEST_OBJS:.o=.d)

unit-tests.exe: $(UNIT_TEST_OBJS) $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(UNIT_TEST_OBJS) $(LIBS)

all: unit-tests.exe


# Rule for tests that have dedicated .cc files, which is what I
# would like to transition toward.
#
# Well, I would like to transition *away* from having tests inside the
# main .cc file.  But, preferably, tests are run from the main unit test
# program rather than as stand-alone programs.
#
test-%.exe: test-%.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) test-$*.cc $(LIBS)

# Same rule but for the other way of naming, which I am slowly adopting.
%-test.exe: %-test.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $*-test.cc $(LIBS)


# Create a read-only file I can try to inspect in test-sm-file-util.cc.
check: test.dir/read-only.txt
test.dir/read-only.txt:
	mkdir -p test.dir
	echo "this file is read-only" >$@
	chmod a-w $@


# If we are missing an expect file, just make an empty one.
test/%.expect:
	touch $@

# Run a single test executable and compare to expected output.
out/%.ok: test/%.expect %.exe
	@mkdir -p $(dir $@)
	$(RUN_COMPARE_EXPECT) \
	  --actual out/$*.actual \
	  --expect test/$*.expect \
	  ./$*.exe
	touch $@

check: out/boxprint.ok
check: out/test-tree-print.ok


# ------------------------- binary-stdin-test --------------------------
all: binary-stdin-test.exe

out/binary-stdin-test.ok: binary-stdin-test.exe
	@mkdir -p $(dir $@)
	@#
	@# Generate a file with all 256 bytes.
	./binary-stdin-test.exe allbytes out/allbytes.bin
	@#
	@# Test reading stdin with 'read'.
	./binary-stdin-test.exe read0 allbytes < out/allbytes.bin
	@#
	@# Test writing stdout with 'write'.
	./binary-stdin-test.exe allbytes write1 > out/allbytes-actual.bin
	cmp out/allbytes-actual.bin out/allbytes.bin
	@#
	@# Test reading stdin with 'fread'.
	./binary-stdin-test.exe fread_stdin allbytes < out/allbytes.bin
	@#
	@# Test writing stdout with 'fwrite'.
	./binary-stdin-test.exe allbytes fwrite_stdout > out/allbytes-actual.bin
	cmp out/allbytes-actual.bin out/allbytes.bin
	@#
	@# Test reading stdin with 'cin.read'.
	./binary-stdin-test.exe cin_read allbytes < out/allbytes.bin
	@#
	@# Test writing stdout with 'cout.write'
	./binary-stdin-test.exe allbytes cout_write > out/allbytes-actual.bin
	cmp out/allbytes-actual.bin out/allbytes.bin
	@#
	@# Done.
	touch $@

check: out/binary-stdin-test.ok


# ------------------------------- check --------------------------------
ifneq ($(CROSS_COMPILE),1)
  RUN :=
else
  # there is a necessary space at the end of the next line ...
  RUN := true 
endif

# for now, check-full is just check
.PHONY: check-full
check-full: check

out/unit-tests.exe.ok: unit-tests.exe
	$(CREATE_OUTPUT_DIRECTORY)
	./unit-tests.exe
	touch $@

check: out/unit-tests.exe.ok

check: $(TESTS)
	$(RUN)./nonport.exe
	$(RUN)./cycles.exe
	$(RUN)./hashline.exe
	$(RUN)./srcloc.exe
	$(RUN)./gprintf.exe
	$(RUN)./vptrmap.exe
	$(RUN)./pprint.exe
	$(RUN)./tarrayqueue.exe
	$(RUN)./testarray.exe
	$(RUN)./bitarray.exe
	$(RUN)./d2vector.exe
	$(RUN)./bdffont.exe
	$(RUN)./tarray2d.exe
	$(RUN)./test-codepoint.exe
	$(RUN)./test-datetime.exe
	$(RUN)./test-owner.exe
	$(RUN)./test-refct-serf.exe
	$(RUN)./test-run-process.exe --unit-test
	$(RUN)./test-sm-file-util.exe
	$(RUN)./test-stringset.exe
ifneq ($(CROSS_COMPILE),1)
	@echo
	@echo "make check: all the tests PASSED"
else
	@echo
	@echo "make check: all the test programs were built, but I did not"
	@echo "try to run any of them because of cross-compile mode; you"
	@echo "may want to try running the above commands yourself on the target"
	@echo "(remove the 'true' prefixes)"
endif


# ------------------- test run-compare-expect.py -----------------------
test/rce_expect_%:
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


# --------------------------- Documentation ----------------------------
# directory of generated documentation
gendoc:
	mkdir gendoc

# main dependencies for the library; some ubiquitous dependencies
# are omitted to avoid too much clutter; the files listed below are
# the roots of the dependency exploration; I don't include any of
# the stand-alone programs since those are just clutter to someone
# trying to understand the library's structure
.PHONY: gendoc/dependencies.dot
gendoc/dependencies.dot:
	perl ./scan-depends.pl -r -Xxassert.h -Xtest.h -Xtyp.h -Xmacros.h -Xstr.h \
		-Xbreaker.h \
		growbuf.h objpool.h strhash.h voidlist.h svdict.h str.h \
		warn.cpp mysig.h srcloc.cc hashline.cc astlist.h taillist.h \
		objstack.h ohashtbl.h okhasharr.h okhashtbl.h sobjlist.h \
		exc.h >$@

# check to see if they have dot
.PHONY: dot
dot:
	@if ! which dot >$(DEV_NULL); then \
	  echo "You don't have the 'dot' tool.  You can get it at:"; \
	  echo "http://www.research.att.com/sw/tools/graphviz/"; \
	  exit 2; \
	fi

# use 'dot' to lay out the graph
%.ps: %.dot dot
	dot -Tps <$*.dot >$@

# use 'convert' to make a PNG image with resolution not to exceed
# 1000 in X or 700 in Y ('convert' will preserve aspect ratio); this
# also antialiases, so it looks very nice (it's hard to reproduce
# this using 'gs' alone)
%.png: %.ps
	convert -geometry 1000x700 $^ $@

# build auto-generated documentation
.PHONY: doc
doc: gendoc gendoc/dependencies.png
	@echo "built documentation"


# ----------------------------- coverage -------------------------------
# Run gcov to produce .gcov files.  This requires having compiled with
# COVERAGE=1.
.PHONY: run-gcov
run-gcov:
	$(GCOV) $(SRCS)

# Clean up previous gcov output.
.PHONY: gcov-clean
gcov-clean:
	$(RM) *.gcov *.gcda *.gcno


# ----------------------- compile_commands.json ------------------------
# Claim this is "phony" so we can regenerate with just "make
# compile_commands.json".  Also, I do not clean this file so I can make
# it using clang then switch back to gcc.
.PHONY: compile_commands.json

# This requires that the build was run with Clang and
# CREATE_O_JSON_FILES.
compile_commands.json:
	(echo "["; cat *.o.json; echo "]") > $@


# ------------------------------- clean --------------------------------
# delete compiling/editing byproducts
clean: gcov-clean
	rm -f *.o *.o.json *~ *.a *.d *.exe gmon.out srcloc.tmp testcout flattest.tmp
	rm -rf test.dir out

distclean: clean
	rm -f config.mk compile_commands.json
	rm -rf gendoc

# remove crap that vc makes
vc-clean:
	rm -f *.plg *.[ip]db *.pch


# EOF
