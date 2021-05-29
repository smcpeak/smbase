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

# Extension for executables, if any, including the ".".
EXE = .exe

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
TESTS += autofile
TESTS += bdffont
TESTS += bflatten
TESTS += bit2d
TESTS += bitarray
TESTS += boxprint
TESTS += crc
TESTS += cycles
TESTS += d2vector
TESTS += datablok
TESTS += gprintf
TESTS += growbuf
TESTS += hashline
TESTS += mypopen
TESTS += mysig
TESTS += nonport
TESTS += pprint
TESTS += smregexp
TESTS += srcloc
TESTS += str
TESTS += strdict
TESTS += strhash
TESTS += strutil
TESTS += svdict
TESTS += taillist_test
TESTS += tarray2d
TESTS += tarrayqueue
TESTS += test-codepoint
TESTS += test-datetime
TESTS += test-refct-serf
TESTS += test-sm-file-util
TESTS += test-stringset
TESTS += testarray
TESTS += testmalloc
TESTS += tobjlist
TESTS += tobjpool
TESTS += trdelete
TESTS += tsobjlist
TESTS += unit-tests
TESTS += vdtllist
TESTS += voidlist
TESTS += vptrmap

# Some programs do not build on Mingw.
ifeq ($(TARGET_PLATFORM_IS_MINGW),1)
  TESTS := $(filter-out testmalloc mypopen mysig smregexp,$(TESTS))
endif

TESTS := $(addsuffix $(EXE),$(TESTS))

tests: $(TESTS)


# command to compile and link
TESTCC  := $(CC) -g -Wall
TESTCXX := $(CXX) -g -Wall

# this goes at the end of most commands that build a test binary
TESTFLAGS := $(CCFLAGS) $(LDFLAGS)

# this one is explicitly *not* linked against $(THIS)
nonport$(EXE): nonport.cpp nonport.h gprintf.o
	$(TESTCXX) -o $@ -DTEST_NONPORT nonport.cpp gprintf.o $(CCFLAGS)

voidlist$(EXE): voidlist.cc voidlist.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_VOIDLIST voidlist.cc $(TESTFLAGS)

vdtllist$(EXE): vdtllist.cc vdtllist.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_VDTLLIST vdtllist.cc $(TESTFLAGS)

taillist_test$(EXE): taillist_test.cc taillist.h $(THIS)
	$(TESTCXX) -o $@ taillist_test.cc $(TESTFLAGS)

tobjlist$(EXE): tobjlist.cc objlist.h voidlist.o $(THIS)
	$(TESTCXX) -o $@ tobjlist.cc voidlist.o $(TESTFLAGS)

tsobjlist$(EXE): tsobjlist.cc sobjlist.h voidlist.o $(THIS)
	$(TESTCXX) -o $@ tsobjlist.cc voidlist.o $(TESTFLAGS)

bit2d$(EXE): bit2d.cc bit2d.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_BIT2D bit2d.cc $(TESTFLAGS)

growbuf$(EXE): growbuf.cc growbuf.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_GROWBUF growbuf.cc $(TESTFLAGS)

strdict$(EXE): strdict.cc strdict.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STRDICT strdict.cc $(TESTFLAGS)

svdict$(EXE): svdict.cc svdict.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_SVDICT svdict.cc $(TESTFLAGS)

str$(EXE): str.cpp str.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STR str.cpp $(TESTFLAGS)

strutil$(EXE): strutil.cc strutil.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STRUTIL strutil.cc $(TESTFLAGS)

strhash$(EXE): strhash.cc strhash.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_STRHASH strhash.cc $(TESTFLAGS)

trdelete$(EXE): trdelete.cc trdelete.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_TRDELETE trdelete.cc $(TESTFLAGS)

bflatten$(EXE): bflatten.cc bflatten.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_BFLATTEN bflatten.cc $(TESTFLAGS)

mysig$(EXE): mysig.cc mysig.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_MYSIG mysig.cc $(TESTFLAGS)

testmalloc$(EXE): testmalloc.cc $(THIS)
	echo TESTS is $(TESTS)
	$(TESTCXX) -o $@ testmalloc.cc $(TESTFLAGS)

mypopen$(EXE): mypopen.c mypopen.h
	$(TESTCC) -o $@ -DTEST_MYPOPEN mypopen.c

# this test is only useful when malloc is compiled with DEBUG_HEAP
tmalloc$(EXE): tmalloc.c libsmbase.a
	$(TESTCC) -o $@ tmalloc.c $(TESTFLAGS)

tobjpool$(EXE): tobjpool.cc objpool.h
	$(TESTCXX) -o $@ tobjpool.cc $(TESTFLAGS)

cycles$(EXE): cycles.h cycles.c
	$(TESTCC) -o $@ -DTEST_CYCLES cycles.c

crc$(EXE): crc.cpp
	$(TESTCXX) -o $@ $(CCFLAGS) -DTEST_CRC crc.cpp

srcloc$(EXE): srcloc.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_SRCLOC srcloc.cc $(TESTFLAGS)

hashline$(EXE): hashline.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_HASHLINE hashline.cc $(TESTFLAGS)

gprintf$(EXE): gprintf.c gprintf.h
	$(TESTCC) -o $@ -DTEST_GPRINTF gprintf.c $(CFLAGS)

smregexp$(EXE): smregexp.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_SMREGEXP smregexp.cc $(TESTFLAGS)

vptrmap$(EXE): vptrmap.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_VPTRMAP vptrmap.cc $(TESTFLAGS)

pprint$(EXE): pprint.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_PPRINT pprint.cc $(TESTFLAGS)

boxprint$(EXE): boxprint.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_BOXPRINT boxprint.cc $(TESTFLAGS)

tarrayqueue$(EXE): tarrayqueue.cc $(THIS)
	$(TESTCXX) -o $@ tarrayqueue.cc $(TESTFLAGS)

testarray$(EXE): testarray.cc $(THIS)
	$(TESTCXX) -o $@ testarray.cc $(TESTFLAGS)

autofile$(EXE): autofile.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_AUTOFILE autofile.cc $(TESTFLAGS)

bitarray$(EXE): bitarray.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_BITARRAY bitarray.cc $(TESTFLAGS)

d2vector$(EXE): d2vector.c $(THIS)
	$(TESTCXX) -o $@ -DTEST_D2VECTOR d2vector.c $(TESTFLAGS)

bdffont$(EXE): bdffont.cc $(THIS)
	$(TESTCXX) -o $@ -DTEST_BDFFONT bdffont.cc $(TESTFLAGS)

tarray2d$(EXE): tarray2d.cc array2d.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_TARRAY2D tarray2d.cc $(TESTFLAGS)

datablok$(EXE): datablok.cpp datablok.h $(THIS)
	$(TESTCXX) -o $@ -DTEST_DATABLOK datablok.cpp $(TESTFLAGS)

UNIT_TEST_OBJS :=
UNIT_TEST_OBJS += test-dict.o
UNIT_TEST_OBJS += test-overflow.o
UNIT_TEST_OBJS += test-parsestring.o
UNIT_TEST_OBJS += unit-tests.o

-include $(UNIT_TEST_OBJS:.o=.d)

unit-tests$(EXE): $(UNIT_TEST_OBJS) $(THIS)
	$(TESTCXX) -o $@ $(UNIT_TEST_OBJS) $(TESTFLAGS)

all: unit-tests$(EXE)

# Rule for tests that have dedicated .cc files, which is what I
# would like to transition toward.
test-%$(EXE): test-%.cc $(THIS)
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
	$(RUN)./nonport$(EXE)
	$(RUN)./voidlist$(EXE)
	$(RUN)./vdtllist$(EXE)
	$(RUN)./tobjlist$(EXE)
	$(RUN)./bit2d$(EXE)
	$(RUN)./growbuf$(EXE)
	$(RUN)./strdict$(EXE)
	$(RUN)./svdict$(EXE)
	$(RUN)./str$(EXE)
	$(RUN)./strutil$(EXE)
	$(RUN)./strhash$(EXE)
	$(RUN)./trdelete$(EXE)
	$(RUN)./bflatten$(EXE)
	$(RUN)./tobjpool$(EXE)
	$(RUN)./cycles$(EXE)
	$(RUN)./tsobjlist$(EXE)
	$(RUN)./hashline$(EXE)
	$(RUN)./srcloc$(EXE)
	$(RUN)./gprintf$(EXE)
	$(RUN)./vptrmap$(EXE)
	$(RUN)./pprint$(EXE)
	$(RUN)./boxprint$(EXE)
	$(RUN)./tarrayqueue$(EXE)
	$(RUN)./testarray$(EXE)
	$(RUN)./taillist_test$(EXE)
	$(RUN)./autofile$(EXE) autofile.cc
	$(RUN)./bitarray$(EXE)
	$(RUN)./d2vector$(EXE)
	$(RUN)./bdffont$(EXE)
	$(RUN)./tarray2d$(EXE)
	$(RUN)./test-codepoint$(EXE)
	$(RUN)./test-datetime$(EXE)
	$(RUN)./test-refct-serf$(EXE)
	$(RUN)./test-sm-file-util$(EXE)
	$(RUN)./test-stringset$(EXE)
	$(RUN)./datablok$(EXE)
	$(RUN)./unit-tests$(EXE)
ifneq ($(TARGET_PLATFORM_IS_MINGW),1)
	$(RUN)./mysig$(EXE)
	$(RUN)./testmalloc$(EXE) >/dev/null 2>&1
	$(RUN)./mypopen$(EXE)
	$(RUN)./smregexp$(EXE)
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
	rm -f *.o *~ *.d gmon.out srcloc.tmp testcout
	rm -f $(TESTS)
	rm -f *.a
	rm -rf test.dir

distclean: clean
	rm -rf gendoc

# remove crap that vc makes
vc-clean:
	rm -f *.plg *.[ip]db *.pch


# end of Makefile
