// ofstreamts.cc
// Code for `ofstreamts` module.

// quarl 2006-05-25 initial version, factored from 2006-05-16 astgen.cc

#include "ofstreamts.h"                // this module

#include "exc.h"                       // smbase::xfatal
#include "sm-file-util.h"              // SMFileUtil

using namespace smbase;


size_t getFileSize(istream &i)
{
  // use 'streampos' instead of 'istream::pos_type' for gcc-2.95
  // compatibility.
  if (!i) return 0;
  streampos save_pos = i.tellg();
  i.seekg(0, ios::beg);
  streampos begin_pos = i.tellg();
  i.seekg(0, ios::end);
  streampos end_pos = i.tellg();
  size_t size = end_pos - begin_pos;
  i.seekg(save_pos);
  return size;
}

bool filesIdentical(const char *f1, const char *f2)
{
  ifstream i1(f1);
  if (!i1) return false;

  ifstream i2(f2);
  if (!i2) {
    xfatal(stringb("I thought I just wrote " << f2 << ", but it doesn't exist"));
    return false;
  }

  if (getFileSize(i1) != getFileSize(i2))
    return false;

  while (true) {
    if (i1.bad() || i2.bad()) {
      return false;
    }

    if (i1.eof() && i2.eof()) {
      // reached EOF and no problem.
      return true;
    }

    if (!i1 || !i2) {
      // Other error? or for some reason we reached EOF in one file but not
      // another even though file sizes are the same
      return false;
    }

    if (i1.get() != i2.get()) {
      // unidentical character
      return false;
    }
  }
}

const char *ofstreamTS::init_fname(string const &destFname0)
{
  destFname = destFname0;
  tmpFname = destFname0 + ".tmp";
  return tmpFname.c_str();
}

void ofstreamTS::save() {
  SMFileUtil sfu;

  close();
  if (filesIdentical(destFname.c_str(), tmpFname.c_str())) {
    cout << "  file " << destFname << " unchanged, so not overwriting it.\n";
    sfu.removeFile(tmpFname);
  }
  else {
    sfu.atomicallyRenameFile(tmpFname, destFname);
  }
}

void ofstreamTS::deleteTmp() {
  SMFileUtil sfu;

  close();
  sfu.removeFile(tmpFname);
}
