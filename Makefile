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

# Flags to control compiler warnings.
WARNING_FLAGS =

# Flags to control optimization.
OPTIMIZATION_FLAGS = -O2

# Flag for C++ standard to use.
CXX_STD_FLAGS = -std=c++11

# Flags for the C and C++ compilers (and preprocessor),
CFLAGS  = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS)
CCFLAGS = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(CXX_STD_FLAGS) $(WARNING_FLAGS)

# Libraries to link with when creating test executables.
LIBS = $(THIS)

# Flags for the linker.
LDFLAGS = $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(LIBS)

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


# compile .cc to .o
%.o: %.cc
	$(CXX) -c -o $@ $(CCFLAGS) $<
	@perl ./depend.pl -o $@ $(CCFLAGS) $< > $*.d

%.o: %.cpp
	$(CXX) -c -o $@ $(CCFLAGS) $<
	@perl ./depend.pl -o $@ $(CCFLAGS) $< > $*.d

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	@perl ./depend.pl -o $@ $(CFLAGS) $< > $*.d


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
MALLOC_CCFLAGS := -O3

# By default, compile+link a stub module that does nothing, so that
# we will just use the normal system malloc.  Only if the user wants
# special malloc features will we switch to Doug Lea's.  The reason
# is I've only tested my extra features on Linux, and on some other
# systems (cygwin, OSX) they don't work and I don't have the inclination
# to fix all my hacks.
MALLOC_MODULE := malloc_stub

# debug version (much slower, but *great* for finding memory errors)
ifeq ($(DEBUG_HEAP),1)
  MALLOC_CCFLAGS := -DDEBUG -DDEBUG_HEAP
  MALLOC_MODULE := malloc
endif

# tracing messages
ifeq ($(TRACE_HEAP),1)
  MALLOC_CCFLAGS += -DTRACE_MALLOC_CALLS
  MALLOC_MODULE := malloc
endif

$(MALLOC_MODULE).o: $(MALLOC_MODULE).c
	$(CC) -c -g $(MALLOC_CCFLAGS) $(MALLOC_MODULE).c


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


# command to compile and link
TESTCC  := $(CC) -g -Wall
TESTCXX := $(CXX) -g -Wall

# this goes at the end of most commands that build a test binary
TESTFLAGS := $(CCFLAGS) $(LDFLAGS)

# this one is explicitly *not* linked against $(THIS)
nonport.exe: nonport.cpp nonport.h gprintf.o
	$(TESTCXX) -o $@ -DTEST_NONPORT nonport.cpp gprintf.o $(CCFLAGS)

voidlist.exe: voidlist.cc voidlist.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_VOIDLIST voidlist.cc $(TESTFLAGS)

vdtllist.exe: vdtllist.cc vdtllist.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_VDTLLIST vdtllist.cc $(TESTFLAGS)

taillist_test.exe: taillist_test.cc taillist.h $(THIS)
	$(TESTCXX) -o $@ taillist_test.cc $(TESTFLAGS)

tobjlist.exe: tobjlist.cc objlist.h voidlist.o $(THIS)
	$(TESTCXX) -o $@ tobjlist.cc voidlist.o $(TESTFLAGS)

tsobjlist.exe: tsobjlist.cc sobjlist.h voidlist.o $(THIS)
	$(TESTCXX) -o $@ tsobjlist.cc voidlist.o $(TESTFLAGS)

bit2d.exe: bit2d.cc bit2d.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_BIT2D bit2d.cc $(TESTFLAGS)

growbuf.exe: growbuf.cc growbuf.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_GROWBUF growbuf.cc $(TESTFLAGS)

strdict.exe: strdict.cc strdict.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STRDICT strdict.cc $(TESTFLAGS)

svdict.exe: svdict.cc svdict.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_SVDICT svdict.cc $(TESTFLAGS)

str.exe: str.cpp str.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STR str.cpp $(TESTFLAGS)

strutil.exe: strutil.cc strutil.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STRUTIL strutil.cc $(TESTFLAGS)

strhash.exe: strhash.cc strhash.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STRHASH strhash.cc $(TESTFLAGS)

trdelete.exe: trdelete.cc trdelete.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_TRDELETE trdelete.cc $(TESTFLAGS)

bflatten.exe: bflatten.cc bflatten.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_BFLATTEN bflatten.cc $(TESTFLAGS)

mysig.exe: mysig.cc mysig.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_MYSIG mysig.cc $(TESTFLAGS)

testmalloc.exe: testmalloc.cc $(THIS)
	echo TESTS is $(TESTS)
	$(TESTCXX) -o $@ testmalloc.cc $(TESTFLAGS)

mypopen.exe: mypopen.c mypopen.h
	$(TESTCC) -o $@ -DTEST_MYPOPEN mypopen.c

# this test is only useful when malloc is compiled with DEBUG_HEAP
tmalloc.exe: tmalloc.c libsmbase.a
	$(TESTCC) -o $@ tmalloc.c $(TESTFLAGS)

tobjpool.exe: tobjpool.cc objpool.h
	$(TESTCXX) -o $@ tobjpool.cc $(TESTFLAGS)

cycles.exe: cycles.h cycles.c
	$(TESTCC) -o $@ -DTEST_CYCLES cycles.c

crc.exe: crc.cpp
	$(TESTCXX) -o $@ $(CCFLAGS) -DTEST_CRC crc.cpp

srcloc.exe: srcloc.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_SRCLOC srcloc.cc $(TESTFLAGS)

hashline.exe: hashline.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_HASHLINE hashline.cc $(TESTFLAGS)

gprintf.exe: gprintf.c gprintf.h
	$(TESTCC) -o $@ -DTEST_GPRINTF gprintf.c $(CFLAGS)

smregexp.exe: smregexp.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_SMREGEXP smregexp.cc $(TESTFLAGS)

vptrmap.exe: vptrmap.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_VPTRMAP vptrmap.cc $(TESTFLAGS)

pprint.exe: pprint.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_PPRINT pprint.cc $(TESTFLAGS)

boxprint.exe: boxprint.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_BOXPRINT boxprint.cc $(TESTFLAGS)

tarrayqueue.exe: tarrayqueue.cc $(THIS)
	$(TESTCXX) -o $@ tarrayqueue.cc $(TESTFLAGS)

testarray.exe: testarray.cc $(THIS)
	$(TESTCXX) -o $@ testarray.cc $(TESTFLAGS)

autofile.exe: autofile.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_AUTOFILE autofile.cc $(TESTFLAGS)

bitarray.exe: bitarray.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_BITARRAY bitarray.cc $(TESTFLAGS)

d2vector.exe: d2vector.c $(THIS)
	$(TESTCXX) -o $@ -DTEST_D2VECTOR d2vector.c $(TESTFLAGS)

bdffont.exe: bdffont.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_BDFFONT bdffont.cc $(TESTFLAGS)

tarray2d.exe: tarray2d.cc array2d.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_TARRAY2D tarray2d.cc $(TESTFLAGS)

datablok.exe: datablok.cpp datablok.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_DATABLOK datablok.cpp $(TESTFLAGS)

UNIT_TEST_OBJS :=
UNIT_TEST_OBJS += test-dict.o
UNIT_TEST_OBJS += test-overflow.o
UNIT_TEST_OBJS += test-parsestring.o
UNIT_TEST_OBJS += unit-tests.o

-include $(UNIT_TEST_OBJS:.o=.d)

unit-tests.exe: $(UNIT_TEST_OBJS) $(THIS)
	$(TESTCXX) -o $@ $(UNIT_TEST_OBJS) $(TESTFLAGS)

all: unit-tests.exe

# Rule for tests that have dedicated .cc files, which is what I
# would like to transition toward.
test-%.exe: test-%.cc $(THIS)
	$(TESTCXX) -o $@ test-$*.cc $(TESTFLAGS)


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
