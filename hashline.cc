// hashline.cc
// code for hashline.h

#include "hashline.h"      // this module

#include <string.h>        // memcpy


HashLineMap::HashLineMap(rostring pf)
  : ppFname(pf),
    filenames(),     // empty
    directives(),    // empty
    prev_ppLine(-1)  // user shouldn't have negative line numbers
{}


HashLineMap::~HashLineMap()
{}

char const *HashLineMap::canonizeFilename(char const *fname)
{
  // map 'fname' to a canonical reference
  return filenames(fname);
}

void HashLineMap::addHashLine(int ppLine, int origLine, char const *origFname)
{
  // check that entries are being added in sorted order
  xassert(ppLine > prev_ppLine);
  prev_ppLine = ppLine;

  // map 'origFname' to a canonical reference
  origFname = canonizeFilename(origFname);

  // add the entry to the array
  directives.push(HashLine(ppLine, origLine, origFname));
}


void HashLineMap::doneAdding()
{
  // printf("## HashLineMap::doneAdding: directives.length() = %d, directives.size() = %d\n",
  //        directives.length(), directives.size());
  directives.setAllocatedSize(directives.length());

  // // make a new array of exactly the right size
  // ArrayStack<HashLine> tmp(directives.length());

  // // copy all the entries into the new array
  // memcpy(tmp.getDangerousWritableArray(), directives.getArray(),
  //        directives.length() * sizeof(HashLine));
  // tmp.setLength(directives.length());

  // // swap the internal contents of the two arrays, so 'directives'
  // // becomes the consolidated one
  // tmp.swapWith(directives);

  // // now tmp's internal storage will be automatically deleted
}


// for queries exactly on #line directives we return the specified
// origLine minus 1, but I don't specify any behavior in that case
// so it's not a problem
void HashLineMap::map(int ppLine, int &origLine, char const *&origFname) const
{
  // check for a ppLine that precedes any in the array
  if (directives.isEmpty() ||
      ppLine < directives[0].ppLine) {
    // it simply refers to the pp file
    origLine = ppLine;
    origFname = ppFname.c_str();
    return;
  }

  // perform binary search on the 'directives' array
  int low = 0;                        // index of lowest candidate
  int high = directives.length()-1;   // index of highest candidate

  while (low < high) {
    // check the midpoint (round up to ensure progress when low+1 == high)
    int mid = (low+high+1)/2;
    if (directives[mid].ppLine > ppLine) {
      // too high
      high = mid-1;
    }
    else {
      // too low or just right
      low = mid;
    }
  }
  xassert(low == high);
  HashLine const &hl = directives[low];

  // the original line is the origLine in the array entry, plus the
  // offset between the ppLine passed in and the ppLine in the array,
  // minus 1 because the #line directive itself occupies one pp line
  origLine = hl.origLine + (ppLine - hl.ppLine - 1);

  origFname = hl.origFname;
}


int HashLineMap::mapLine(int ppLine) const
{
  int origLine;
  char const *origFname;
  map(ppLine, origLine, origFname);
  return origLine;
}

char const *HashLineMap::mapFile(int ppLine) const
{
  int origLine;
  char const *origFname;
  map(ppLine, origLine, origFname);
  return origFname;
}


// EOF
