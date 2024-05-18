// strhash-test.cc
// Tests for strhash.

#include "strhash.h"     // module under test

#include "array.h"       // GrowArray
#include "crc.h"         // crc32
#include "nonport.h"     // getMilliseconds
#include "sm-iostream.h" // cout
#include "str.h"         // OldSmbaseString
#include "trace.h"       // traceProgress
#include "xassert.h"     // xassert

#include <fstream>       // filebuf
#include <iostream>      // istream
#include <stdlib.h>      // rand


namespace {


// pair a GrowArray with its size
struct StringArray {
  int tableSize;
  GrowArray<char*> table;
  bool appendable;

  StringArray(int tableSize0)
    : tableSize(tableSize0)
    , table(tableSize)
    , appendable(tableSize == 0)
  {}
  void append(char *str) {
    xassert(appendable);
    table.ensureIndexDoubler(tableSize);
    table[tableSize] = str;
    ++tableSize;
  }
};

// data to hash
StringArray *dataArray = NULL;


char const *id(void *p)
{
  return (char const*)p;
}

char *randomString()
{
  char *ret = new char[11];
  loopi(10) {
    ret[i] = (rand()%26)+'a';
  }
  ret[10]=0;
  return ret;
}

// fill a table with random strings
void makeRandomData(int numRandStrs) {
  dataArray = new StringArray(numRandStrs);
  {loopi(dataArray->tableSize) {
    dataArray->table[i] = randomString();
  }}
}


// file the data array with whitespace-delimited strings from a file
void readDataFromFile(char *inFileName) {
  dataArray = new StringArray(0);
  char const *delim = " \t\n\r\v\f";
  std::filebuf fb;
  fb.open (inFileName, ios::in);
  istream in(&fb);
  while(true) {
    stringBuilder s;
    s.readdelim(in, delim);
//      cout << ":" << s->pcharc() << ":" << endl;
    if (in.eof()) break;
//      // don't insert 0 length strings
//      if (s->length() == 0) continue;
    dataArray->append(strdup(s.c_str()));
  }
}

void writeData(ostream &out) {
  cout << "write data" << endl;
  for(int i=0; i<dataArray->tableSize; ++i) {
    out << dataArray->table[i] << endl;
  }
}

// dsw: what is the point of this?
// dealloc the test strings
//  void deleteData() {
//    {loopi(dataArray->tableSize) {
//      delete[] dataArray->table[i];
//    }}
//  //    delete[] dataArray->table;
//  }

void correctnessTest() {
  traceProgress() << "start of strhash correctness testing\n";

  // insert them all into a hash table
  StringHash hash(id);
  {loopi(dataArray->tableSize) {
    hash.add(dataArray->table[i], dataArray->table[i]);
    hash.selfCheck();
  }}
  hash.selfCheck();
  xassert(hash.getNumEntries() == dataArray->tableSize);

  // verify that they are all mapped properly
  {loopi(dataArray->tableSize) {
    xassert(hash.get(dataArray->table[i]) == dataArray->table[i]);
  }}
  hash.selfCheck();

  // remove every other one
  {loopi(dataArray->tableSize) {
    if (i%2 == 0) {
      hash.remove(dataArray->table[i]);
      hash.selfCheck();
    }
  }}
  hash.selfCheck();
  xassert(hash.getNumEntries() == dataArray->tableSize / 2);

  // verify it
  {loopi(dataArray->tableSize) {
    if (i%2 == 0) {
      xassert(hash.get(dataArray->table[i]) == NULL);
    }
    else {
      xassert(hash.get(dataArray->table[i]) == dataArray->table[i]);
    }
  }}
  hash.selfCheck();

  // remove the rest
  {loopi(dataArray->tableSize) {
    if (i%2 == 1) {
      hash.remove(dataArray->table[i]);
      hash.selfCheck();
    }
  }}
  hash.selfCheck();
  xassert(hash.getNumEntries() == 0);

  traceProgress() << "end of strhash correctness testing\n";
}

void performanceTest(int numPerfRuns) {
  // test performance of the hash function
  traceProgress() << "start of strhash performance testing\n";

  long startTime = getMilliseconds();
  loopj(numPerfRuns) {
    loopi(dataArray->tableSize) {
      StringHash::coreHash(dataArray->table[i]);
      //crc32((unsigned char*)dataArray->table[i], strlen(dataArray->table[i]));
      //crc32((unsigned char*)dataArray->table[i], 10);
    }
  }
  long stopTime = getMilliseconds();
  long duration = stopTime - startTime;
  cout << "milliseconds to hash: " << duration << endl;

  traceProgress() << "end of strhash performance testing\n";
}

// command-line state
int numRandStrs = 0;
char *inFileName = NULL;
bool dump = false;
bool testCor = true;
bool testPerf = true;
int numPerfRuns = 10000;

void usage() {
  cout << "Test the string hashing module strhash.cc\n"
       << "  --help / -h     : print this message\n"
       << "  --[no-]testCor  : run the correctness tests\n"
       << "                    will fail if data has duplicate strings (?!)\n"
       << "  --[no-]testPerf : run the performance tests\n"
       << "  --numPerfRuns N : loop over data N times during performance run\n"
       << "  --file FILE     : use the whitespace-delimited string contents of FILE\n"
       << "  --random N      : use N internally generated random strings of length 10;\n"
       << "                    N should be even\n"
       << "  --dump          : dump out the data after generating/reading it\n"
       << "The default is '--random 300 --testCor --testPerf --numPerfRuns 10000'."
       << endl;
}

void initFromFlags(int &argc, char const **&argv) {
  --argc; ++argv;
  for(;
      *argv;
      --argc, ++argv) {
    if (strcmp(*argv, "--help")==0 || strcmp(*argv, "-h")==0) {
      usage();
      exit(0);
    } else if (strcmp(*argv, "--testCor")==0) {
      testCor = true;
    } else if (strcmp(*argv, "--no-testCor")==0) {
      testCor = false;
    } else if (strcmp(*argv, "--testPerf")==0) {
      testPerf = true;
    } else if (strcmp(*argv, "--no-testPerf")==0) {
      testPerf = false;
    } else if (strcmp(*argv, "--random")==0) {
      if (inFileName) {
        cout << "do not use --random and --file together" << endl;
        usage();
        exit(1);
      }
      --argc; ++argv;
      if (!*argv) {
        cout << "supply an argument to --random" << endl;
        usage();
        exit(1);
      }
      numRandStrs = atoi(*argv);
      if (!(numRandStrs > 0)) {
        cout << "argument to --random must be > 0" << endl;
        usage();
        exit(1);
      }
    } else if (strcmp(*argv, "--file")==0) {
      if (numRandStrs) {
        cout << "do not use --random and --file together" << endl;
        usage();
        exit(1);
      }
      --argc; ++argv;
      if (!*argv) {
        cout << "supply an argument to --file" << endl;
        usage();
        exit(1);
      }
      inFileName = strdup(*argv);
      xassert(inFileName);
    } else if (strcmp(*argv, "--numPerfRuns")==0) {
      --argc; ++argv;
      if (!*argv) {
        cout << "supply an argument to --numPerfRuns" << endl;
        usage();
        exit(1);
      }
      numPerfRuns = atoi(*argv);
      if (!(numPerfRuns > 0)) {
        cout << "argument to --numPerfRuns must be > 0" << endl;
        usage();
        exit(1);
      }
    } else if (strcmp(*argv, "--dump")==0) {
      dump = true;
    } else {
      cout << "unrecognized flag " << *argv << endl;
      usage();
      exit(1);
    }
  }
}


} // anonymous namespace


// Called from unit-tests.cc.
void test_strhash()
{
  // Previously, I had this hooked up to the command line, but that's
  // not so easy in my unit test framework, so this is just vestigial.
  int argc = 1;
  char const *argvArray[2] = {"strhash-test", nullptr};
  char const **argv = argvArray;

  traceAddSys("progress");

  switch (strhashAlgorithmCode) {
    case 1:
      cout << "hash function 1: Nelson" << endl;
      break;

    case 2:
      cout << "hash function 2: word-rotate/final-mix" << endl;
      break;

    default:
      cout << "invalid hash function code!" << endl;
      break;
  }

  // read command line flags
  initFromFlags(argc, argv);

  // read data
  if ((!inFileName) && (!numRandStrs)) {
    numRandStrs = 300;          // default
  }
  if (numRandStrs % 2 != 0) {
    cout << "use an even-number argument for --random" << endl;
    usage();
    exit(1);
  }
  if (numRandStrs) {
    makeRandomData(numRandStrs);
  } else if (inFileName) {
    if (testCor) {
      cout << "Warning: The correctness test fails if strings are duplicated "
        "and you are reading data from a file." << endl;
    }
    readDataFromFile(inFileName);
  } else {
    xfailure("goink?");
  }

  // dump data
  if (dump) {
    writeData(cout);
  }

  // test
  if (testCor) {
    correctnessTest();
  }
  if (testPerf) {
    performanceTest(numPerfRuns);
  }

  // delete data
//    deleteData();

  cout << "strhash tests finished\n";
}


// EOF
