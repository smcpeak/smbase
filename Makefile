# Makefile for libsmbase
# see license.txt for copyright and terms of use

# main target
THIS := libsmbase.a
all: gensrc $(THIS)


# C preprocess, compiler and linker
CC := gcc

# C++ compiler, etc.
CXX := g++

# flags for the C and C++ compilers (and preprocessor)
CCFLAGS := -g -Wall -D__LINUX__ -D__UNIX__

# make warnings into errors so I always get a chance to fix them
CCFLAGS += -Werror

# for gcc-3
CCFLAGS += -Wno-deprecated

# when uncommented, we get profiling info
#CCFLAGS += -pg

# optimizer...
CCFLAGS += -O2 -DNDEBUG

# flags for the linker
LDFLAGS := -g -Wall libsmbase.a


# some other tools
AR     := ar
RANLIB := ranlib


# compile .cc to .o
%.o: %.cc
	$(CXX) -c -o $@ $< $(CCFLAGS)
	@./depend.pl -o $@ $< $(CCFLAGS) > $*.d

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CCFLAGS)
	@./depend.pl -o $@ $< $(CCFLAGS) > $*.d

%.o: %.c
	$(CC) -c -o $@ $< $(CCFLAGS)


# -------- experimenting with m4 for related files -------
# I don't delete these during make clean because I don't want
# to force people to have m4 installed
gensrc: sobjlist.h objlist.h

sobjlist.h: xobjlist.h
	rm -f sobjlist.h
	m4 -Dm4_output=sobjlist.h --prefix-builtins xobjlist.h > sobjlist.h
	chmod a-w sobjlist.h

objlist.h: xobjlist.h
	rm -f objlist.h
	m4 -Dm4_output=objlist.h --prefix-builtins xobjlist.h > objlist.h
	chmod a-w objlist.h


# -------------- main target --------------
# Doug Lea's malloc:
#   add the -DDEBUG flag to turn on doug lea's additional checks
#   add the -DDEBUG_HEAP flag to turn on my zone-based protection
#   add the -DTRACE_MALLOC_CALLS flag to print on every alloc/dealloc
#   normally -O3 is specified
malloc.o: malloc.c
	$(CC) -c -g -O3 -DNO_DEBUG -DNO_TRACE_MALLOC_CALLS -DNO_DEBUG_HEAP malloc.c

# mysig needs some flags to *not* be set ....
mysig.o: mysig.cc mysig.h
	$(CC) -c -g mysig.cc

# library itself
OBJS := \
  bflatten.o \
  bit2d.o \
  bitarray.o \
  breaker.o \
  crc.o \
  cycles.o \
  datablok.o \
  exc.o \
  flatten.o \
  growbuf.o \
  hashtbl.o \
  malloc.o \
  missing.o \
  mypopen.o \
  mysig.o \
  nonport.o \
  point.o \
  srcloc.o \
  str.o \
  strdict.o \
  strhash.o \
  stringset.o \
  strtokp.o \
  strutil.o \
  svdict.o \
  syserr.o \
  trace.o \
  trdelete.o \
  unixutil.o \
  vdtllist.o \
  voidlist.o \
  warn.o
-include $(OBJS:.o=.d)

$(THIS): $(OBJS)
	$(AR) -r $(THIS) $(OBJS)
	$(RANLIB) $(THIS)


# ---------- module tests ----------------
# test program targets
TESTS := nonport voidlist tobjlist bit2d growbuf testmalloc mypopen \
         strdict svdict str strutil trdelete bflatten mysig \
         testmalloc mypopen tobjpool strhash cycles tsobjlist crc \
         srcloc
tests: $(TESTS)

# command to compile and link
TESTCC  := $(CC) -g -Wall
TESTCXX := $(CXX) -g -Wall

# this goes at the end of most commands which builds a test binary
TESTFLAGS := $(CCFLAGS) $(LDFLAGS)

# this one is explicitly *not* linked against $(THIS)
nonport: nonport.cpp nonport.h
	$(TESTCXX) -o nonport -DTEST_NONPORT nonport.cpp $(CCFLAGS)

voidlist: voidlist.cc voidlist.h $(THIS)
	$(TESTCXX) -o voidlist -DTEST_VOIDLIST voidlist.cc $(TESTFLAGS)

tobjlist: tobjlist.cc objlist.h voidlist.o $(THIS)
	$(TESTCXX) -o tobjlist tobjlist.cc voidlist.o $(TESTFLAGS)

tsobjlist: tsobjlist.cc sobjlist.h voidlist.o $(THIS)
	$(TESTCXX) -o $@ tsobjlist.cc voidlist.o $(TESTFLAGS)

bit2d: bit2d.cc bit2d.h $(THIS)
	$(TESTCXX) -o bit2d -DTEST_BIT2D bit2d.cc $(TESTFLAGS)

growbuf: growbuf.cc growbuf.h $(THIS)
	$(TESTCXX) -o growbuf -DTEST_GROWBUF growbuf.cc $(TESTFLAGS)

strdict: strdict.cc strdict.h $(THIS)
	$(TESTCXX) -o strdict -DTEST_STRDICT strdict.cc $(TESTFLAGS)

svdict: svdict.cc svdict.h $(THIS)
	$(TESTCXX) -o svdict -DTEST_SVDICT svdict.cc $(TESTFLAGS)

str: str.cpp str.h $(THIS)
	$(TESTCXX) -o str -DTEST_STR str.cpp $(TESTFLAGS)

strutil: strutil.cc strutil.h $(THIS)
	$(TESTCXX) -o strutil -DTEST_STRUTIL strutil.cc $(TESTFLAGS)

strhash: strhash.cc strhash.h $(THIS)
	$(TESTCXX) -o strhash -DTEST_STRHASH strhash.cc $(TESTFLAGS)

trdelete: trdelete.cc trdelete.h $(THIS)
	$(TESTCXX) -o trdelete -DTEST_TRDELETE trdelete.cc $(TESTFLAGS)

bflatten: bflatten.cc bflatten.h $(THIS)
	$(TESTCXX) -o bflatten -DTEST_BFLATTEN bflatten.cc $(TESTFLAGS)

mysig: mysig.cc mysig.h $(THIS)
	$(TESTCXX) -o mysig -DTEST_MYSIG mysig.cc $(TESTFLAGS)

testmalloc: testmalloc.cc $(THIS)
	$(TESTCXX) -o testmalloc testmalloc.cc $(TESTFLAGS)

mypopen: mypopen.c mypopen.h
	$(TESTCC) -o mypopen -DTEST_MYPOPEN mypopen.c

# this test is only useful when malloc is compiled with DEBUG_HEAP
tmalloc: tmalloc.c
	$(TESTCC) -o tmalloc tmalloc.c $(TESTFLAGS)

tobjpool: tobjpool.cc objpool.h
	$(TESTCXX) -o tobjpool tobjpool.cc $(TESTFLAGS)

cycles: cycles.h cycles.c
	$(TESTCC) -o cycles -DTEST_CYCLES cycles.c

crc: crc.cpp
	$(TESTCXX) -o crc -DTEST_CRC crc.cpp

srcloc: srcloc.cc $(THIS)
	$(TESTCXX) -o srcloc -DTEST_SRCLOC srcloc.cc $(TESTFLAGS)

check: $(TESTS)
	./nonport
	./voidlist
	./tobjlist
	./bit2d
	./growbuf
	./strdict
	./svdict
	./str
	./strutil
	./strhash
	./trdelete
	./bflatten
	./mysig
	./testmalloc >/dev/null 2>&1
	./mypopen
	./tobjpool
	./cycles
	./tsobjlist
	./srcloc
	@echo
	@echo "make check: all the tests PASSED"


# ------------------- documentation -------------------------
# main dependencies for the library; some ubiquitous dependencies
# are omitted to avoid too much clutter
dependencies.dot:
	./scan-depends.pl -r -Xxassert.h -Xtest.h -Xtyp.h -Xmacros.h \
		growbuf.h tobjpool.cc strdict.h voidlist.h svdict.h \
		warn.cpp mysig.h tobjlist.cc >$@

# use 'dot' to lay out the graph
# http://www.research.att.com/sw/tools/graphviz/
%.ps: %.dot
	dot -Tps <$^ >$@

# use 'convert' to make a PNG image with resolution not to exceed
# 1000 in X or 700 in Y ('convert' will preserve aspect ratio); this
# also antialiases, so it looks very nice (it's hard to reproduce
# this using 'gs' alone)
%.png: %.ps
	convert -geometry 1000x700 $^ $@


# --------------------- clean --------------------
# delete compiling/editing byproducts
clean:
	rm -f *.o *~ *.d gmon.out
	rm -f $(TESTS)
	rm -f *.a

# remove crap that vc makes
vc-clean:
	rm -f *.plg *.[ip]db *.pch


# end of Makefile
