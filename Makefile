# smbase/Makefile
# see license.txt for copyright and terms of use

# main target
THIS := libsmbase.a
all: $(THIS)


# ------------------------- Configuration --------------------------
# ---- Running other programs ----
# C preprocessor, compiler and linker.
CC = gcc

# C++ compiler, etc.
CXX = g++

# Flags to control generation of debug info.
DEBUG_FLAGS = -g

# Flags to enable dependency generation of .d files.
GENDEPS_FLAGS = -MMD

# Flags to control optimization.
OPTIMIZATION_FLAGS = -O2

# Flags to control compiler warnings.
WARNING_FLAGS =

# Flags for C or C++ standard to use.
C_STD_FLAGS   = -std=c99
CXX_STD_FLAGS = -std=c++11

# Flags for the C and C++ compilers (and preprocessor).
#
# Note: $(GENDEPS_FLAGS) are not included because these flags are used
# for linking too, and if that used $(GENDEPS_FLAGS) then the .d files
# for .o files would be overwritten with info for .exe files.
CFLAGS   = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(C_STD_FLAGS)
CXXFLAGS = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(CXX_STD_FLAGS)

# Libraries to link with when creating test executables.
LIBS = $(THIS)

# Flags to add to a link command *in addition* to either $(CFLAGS) or
# $(CXXFLAGS), depending on whether C++ modules are included.
LDFLAGS =

# Some other tools.
AR     = ar
RANLIB = ranlib


# ---- Options within this Makefile ----
# Set to 1 if we are building for MinGW.
TARGET_PLATFORM_IS_MINGW = 0

# Set to 1 if we are cross-compiling, meaning the executables we make
# do not run on the build machine.
CROSS_COMPILE = 0

# Set to 1 to activate the rules that generate source code.
GENSRC = 0

# Set to 1 to activate debug heap mechanism.
DEBUG_HEAP = 0

# Set to 1 to activate tracing of heap allocation activity.
TRACE_HEAP = 0


# ---- Automatic Configuration ----
# Pull in settings from ./configure.  They override the defaults above,
# and are in turn overridden by personal.mk, below.
ifeq ($(wildcard config.mk),)
  $(error The file 'config.mk' does not exist.  Run './configure' before 'make'.)
endif
include config.mk


# ---- Customization ----
# Allow customization of the above variables in a separate file.  Just
# create personal.mk with desired settings.
#
# Common things to set during development:
#
#   WERROR = -Werror
#   WARNING_FLAGS = -Wall $(WERROR)
#   OPTIMIZATION_FLAGS =
#
-include personal.mk


# ----------------------------- Rules ------------------------------
# Eliminate all implicit rules.
.SUFFIXES:

# Delete a target when its recipe fails.
.DELETE_ON_ERROR:

# Do not remove "intermediate" targets.
.SECONDARY:


# Compile .cc to .o, also generating dependency files.
%.o: %.cc
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(CXXFLAGS) $<

%.o: %.cpp
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(CXXFLAGS) $<

%.o: %.c
	$(CC) -c -o $@ $(GENDEPS_FLAGS) $(CFLAGS) $<


# -------- experimenting with m4 for related files -------
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


# ----------------------- malloc ------------------------
# Doug Lea's malloc:
#   add the -DDEBUG flag to turn on doug lea's additional checks
#   add the -DDEBUG_HEAP flag to turn on my zone-based protection
#   add the -DTRACE_MALLOC_CALLS flag to print on every alloc/dealloc
#   normally -O3 is specified
MALLOC_CFLAGS := -O3

# By default, compile+link a stub module that does nothing, so that
# we will just use the normal system malloc.  Only if the user wants
# special malloc features will we switch to Doug Lea's.  The reason
# is I've only tested my extra features on Linux, and on some other
# systems (cygwin, OSX) they don't work and I don't have the inclination
# to fix all my hacks.
MALLOC_MODULE := malloc_stub

# debug version (much slower, but *great* for finding memory errors)
ifeq ($(DEBUG_HEAP),1)
  MALLOC_CFLAGS := -DDEBUG -DDEBUG_HEAP
  MALLOC_MODULE := malloc
endif

# tracing messages
ifeq ($(TRACE_HEAP),1)
  MALLOC_CFLAGS += -DTRACE_MALLOC_CALLS
  MALLOC_MODULE := malloc
endif

$(MALLOC_MODULE).o: $(MALLOC_MODULE).c
	$(CC) -c -g $(MALLOC_CFLAGS) $(MALLOC_MODULE).c


# --------------------- main target ---------------------

# mysig needs some flags to *not* be set ....
mysig.o: mysig.cc mysig.h
	$(CXX) -c -g mysig.cc

# library itself
OBJS :=
OBJS += autofile.o
OBJS += bdffont.o
OBJS += bflatten.o
OBJS += bit2d.o
OBJS += bitarray.o
OBJS += boxprint.o
OBJS += breaker.o
OBJS += codepoint.o
OBJS += crc.o
OBJS += cycles.o
OBJS += d2vector.o
OBJS += datablok.o
OBJS += datetime.o
OBJS += dev-warning.o
OBJS += exc.o
OBJS += flatten.o
OBJS += gprintf.o
OBJS += growbuf.o
OBJS += hashline.o
OBJS += hashtbl.o
OBJS += $(MALLOC_MODULE).o
OBJS += missing.o
OBJS += mypopen.o
OBJS += mysig.o
OBJS += nonport.o
OBJS += objcount.o
OBJS += ofstreamts.o
OBJS += parsestring.o
OBJS += point.o
OBJS += pprint.o
OBJS += refct-serf.o
OBJS += sm-file-util.o
OBJS += smregexp.o
OBJS += srcloc.o
OBJS += str.o
OBJS += strdict.o
OBJS += strhash.o
OBJS += stringset.o
OBJS += strtable.o
OBJS += strtokp.o
OBJS += strutil.o
OBJS += svdict.o
OBJS += syserr.o
OBJS += trace.o
OBJS += trdelete.o
OBJS += unixutil.o
OBJS += vdtllist.o
OBJS += vptrmap.o
OBJS += voidlist.o
OBJS += warn.o

# Some modules do not build on Mingw; for the moment I do not need them.
ifeq ($(TARGET_PLATFORM_IS_MINGW),1)
  OBJS := $(filter-out mypopen.o mysig.o smregexp.o,$(OBJS))
endif

# Pull in automatic dependencies created by $(GENDEPS_FLAGS).
-include $(OBJS:.o=.d)

$(THIS): $(OBJS)
	rm -f $(THIS)
	$(AR) -r $(THIS) $(OBJS)
	-$(RANLIB) $(THIS)


# ---------- module tests ----------------
# test program targets
TESTS :=
TESTS += autofile.exe
TESTS += bdffont.exe
TESTS += bflatten.exe
TESTS += bit2d.exe
TESTS += bitarray.exe
TESTS += boxprint.exe
TESTS += crc.exe
TESTS += cycles.exe
TESTS += d2vector.exe
TESTS += datablok.exe
TESTS += gprintf.exe
TESTS += growbuf.exe
TESTS += hashline.exe
TESTS += mypopen.exe
TESTS += mysig.exe
TESTS += nonport.exe
TESTS += pprint.exe
TESTS += smregexp.exe
TESTS += srcloc.exe
TESTS += str.exe
TESTS += strdict.exe
TESTS += strhash.exe
TESTS += strutil.exe
TESTS += svdict.exe
TESTS += taillist_test.exe
TESTS += tarray2d.exe
TESTS += tarrayqueue.exe
TESTS += test-codepoint.exe
TESTS += test-datetime.exe
TESTS += test-owner.exe
TESTS += test-refct-serf.exe
TESTS += test-sm-file-util.exe
TESTS += test-stringset.exe
TESTS += testarray.exe
TESTS += testmalloc.exe
TESTS += tobjlist.exe
TESTS += tobjpool.exe
TESTS += trdelete.exe
TESTS += tsobjlist.exe
TESTS += unit-tests.exe
TESTS += vdtllist.exe
TESTS += voidlist.exe
TESTS += vptrmap.exe

# Some programs do not build on Mingw.
NON_MINGW_TESTS :=
NON_MINGW_TESTS += mypopen.exe
NON_MINGW_TESTS += mysig.exe
NON_MINGW_TESTS += smregexp.exe
NON_MINGW_TESTS += testmalloc.exe

ifeq ($(TARGET_PLATFORM_IS_MINGW),1)
  TESTS := $(filter-out $(NON_MINGW_TESTS),$(TESTS))
endif

tests: $(TESTS)


# this one is explicitly *not* linked against $(THIS)
nonport.exe: nonport.cpp nonport.h gprintf.o
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_NONPORT $(LDFLAGS) nonport.cpp gprintf.o

voidlist.exe: voidlist.cc voidlist.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_VOIDLIST $(LDFLAGS) voidlist.cc $(LIBS)

vdtllist.exe: vdtllist.cc vdtllist.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_VDTLLIST $(LDFLAGS) vdtllist.cc $(LIBS)

taillist_test.exe: taillist_test.cc taillist.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) taillist_test.cc $(LIBS)

tobjlist.exe: tobjlist.cc objlist.h voidlist.o $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) tobjlist.cc voidlist.o $(LIBS)

tsobjlist.exe: tsobjlist.cc sobjlist.h voidlist.o $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) tsobjlist.cc voidlist.o $(LIBS)

bit2d.exe: bit2d.cc bit2d.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BIT2D $(LDFLAGS) bit2d.cc $(LIBS)

growbuf.exe: growbuf.cc growbuf.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_GROWBUF $(LDFLAGS) growbuf.cc $(LIBS)

strdict.exe: strdict.cc strdict.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_STRDICT $(LDFLAGS) strdict.cc $(LIBS)

svdict.exe: svdict.cc svdict.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_SVDICT $(LDFLAGS) svdict.cc $(LIBS)

str.exe: str.cpp str.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_STR $(LDFLAGS) str.cpp $(LIBS)

strutil.exe: strutil.cc strutil.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_STRUTIL $(LDFLAGS) strutil.cc $(LIBS)

strhash.exe: strhash.cc strhash.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_STRHASH $(LDFLAGS) strhash.cc $(LIBS)

trdelete.exe: trdelete.cc trdelete.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_TRDELETE $(LDFLAGS) trdelete.cc $(LIBS)

bflatten.exe: bflatten.cc bflatten.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BFLATTEN $(LDFLAGS) bflatten.cc $(LIBS)

mysig.exe: mysig.cc mysig.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_MYSIG $(LDFLAGS) mysig.cc $(LIBS)

testmalloc.exe: testmalloc.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) testmalloc.cc $(LIBS)

mypopen.exe: mypopen.c mypopen.h
	$(CC) -o $@ $(CFLAGS) -DTEST_MYPOPEN $(LDFLAGS) mypopen.c

# this test is only useful when malloc is compiled with DEBUG_HEAP
tmalloc.exe: tmalloc.c $(THIS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) tmalloc.c $(LIBS)

tobjpool.exe: tobjpool.cc objpool.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) tobjpool.cc $(LIBS)

cycles.exe: cycles.h cycles.c
	$(CC) -o $@ $(CFLAGS) -DTEST_CYCLES $(LDFLAGS) cycles.c

crc.exe: crc.cpp
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_CRC $(LDFLAGS) crc.cpp

srcloc.exe: srcloc.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_SRCLOC $(LDFLAGS) srcloc.cc $(LIBS)

hashline.exe: hashline.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_HASHLINE $(LDFLAGS) hashline.cc $(LIBS)

gprintf.exe: gprintf.c gprintf.h
	$(CC) -o $@ $(CFLAGS) -DTEST_GPRINTF $(LDFLAGS) gprintf.c

smregexp.exe: smregexp.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_SMREGEXP $(LDFLAGS) smregexp.cc $(LIBS)

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

autofile.exe: autofile.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_AUTOFILE $(LDFLAGS) autofile.cc $(LIBS)

bitarray.exe: bitarray.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BITARRAY $(LDFLAGS) bitarray.cc $(LIBS)

d2vector.exe: d2vector.c $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_D2VECTOR $(LDFLAGS) d2vector.c $(LIBS)

bdffont.exe: bdffont.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_BDFFONT $(LDFLAGS) bdffont.cc $(LIBS)

tarray2d.exe: tarray2d.cc array2d.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_TARRAY2D $(LDFLAGS) tarray2d.cc $(LIBS)

datablok.exe: datablok.cc datablok.h $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) -DTEST_DATABLOK $(LDFLAGS) datablok.cc $(LIBS)

UNIT_TEST_OBJS :=
UNIT_TEST_OBJS += test-dict.o
UNIT_TEST_OBJS += test-overflow.o
UNIT_TEST_OBJS += test-parsestring.o
UNIT_TEST_OBJS += unit-tests.o

-include $(UNIT_TEST_OBJS:.o=.d)

unit-tests.exe: $(UNIT_TEST_OBJS) $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(UNIT_TEST_OBJS) $(LIBS)

all: unit-tests.exe

# Rule for tests that have dedicated .cc files, which is what I
# would like to transition toward.
test-%.exe: test-%.cc $(THIS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) test-$*.cc $(LIBS)


# Create a read-only file I can try to inspect in test-sm-file-util.cc.
check: test.dir/read-only.txt
test.dir/read-only.txt:
	mkdir -p test.dir
	echo "this file is read-only" >$@
	chmod a-w $@


ifneq ($(CROSS_COMPILE),1)
  RUN :=
else
  # there is a necessary space at the end of the next line ...
  RUN := true 
endif

# for now, check-full is just check
.PHONY: check-full
check-full: check

check: $(TESTS)
	$(RUN)./nonport.exe
	$(RUN)./voidlist.exe
	$(RUN)./vdtllist.exe
	$(RUN)./tobjlist.exe
	$(RUN)./bit2d.exe
	$(RUN)./growbuf.exe
	$(RUN)./strdict.exe
	$(RUN)./svdict.exe
	$(RUN)./str.exe
	$(RUN)./strutil.exe
	$(RUN)./strhash.exe
	$(RUN)./trdelete.exe
	$(RUN)./bflatten.exe
	$(RUN)./tobjpool.exe
	$(RUN)./cycles.exe
	$(RUN)./tsobjlist.exe
	$(RUN)./hashline.exe
	$(RUN)./srcloc.exe
	$(RUN)./gprintf.exe
	$(RUN)./vptrmap.exe
	$(RUN)./pprint.exe
	$(RUN)./boxprint.exe
	$(RUN)./tarrayqueue.exe
	$(RUN)./testarray.exe
	$(RUN)./taillist_test.exe
	$(RUN)./autofile.exe autofile.cc
	$(RUN)./bitarray.exe
	$(RUN)./d2vector.exe
	$(RUN)./bdffont.exe
	$(RUN)./tarray2d.exe
	$(RUN)./test-codepoint.exe
	$(RUN)./test-datetime.exe
	$(RUN)./test-owner.exe
	$(RUN)./test-refct-serf.exe
	$(RUN)./test-sm-file-util.exe
	$(RUN)./test-stringset.exe
	$(RUN)./datablok.exe
	$(RUN)./unit-tests.exe
ifneq ($(TARGET_PLATFORM_IS_MINGW),1)
	$(RUN)./mysig.exe
	$(RUN)./testmalloc.exe >/dev/null 2>&1
	$(RUN)./mypopen.exe
	$(RUN)./smregexp.exe
endif
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


# ------------------- documentation -------------------------
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
	@if ! which dot >/dev/null; then \
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


# --------------------- clean --------------------
# delete compiling/editing byproducts
clean:
	rm -f *.o *~ *.a *.d *.exe gmon.out srcloc.tmp testcout flattest.tmp
	rm -rf test.dir

distclean: clean
	rm -f config.mk
	rm -rf gendoc

# remove crap that vc makes
vc-clean:
	rm -f *.plg *.[ip]db *.pch


# end of Makefile
