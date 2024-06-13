// temporary-file.cc
// Code for `temporary-file.h`.

#include "temporary-file.h"            // this module

#include "nonport.h"                   // getProcessId
#include "sm-env.h"                    // smbase::envAsBool
#include "sm-file-util.h"              // SMFileUtil
#include "xassert.h"                   // xfailure

#include <stdio.h>                     // std::remove


OPEN_NAMESPACE(smbase)


int TemporaryFile::s_fileNameCounter = 0;


TemporaryFile::~TemporaryFile()
{
  if (!envAsBool("KEEP_TEMPS")) {
    // Ignore failure.
    std::remove(m_fname.c_str());
  }
}


TemporaryFile::TemporaryFile(std::string const &fnamePrefix,
                             std::string const &fnameSuffix,
                             std::string const &contents)
  : m_fname()
{
  SMFileUtil sfu;

  // Loop until we can create the file.
  for (int i=0; i < 1000; ++i) {
    m_fname = stringb(
      fnamePrefix <<
      ".tmp." << getProcessId() <<
      "." << (++s_fileNameCounter) <<
      "." << fnameSuffix);

    // Check if the file exists.  This is a race condition but the
    // alternatives are annoying.
    if (sfu.pathExists(m_fname)) {
      continue;
    }

    sfu.writeFileAsString(m_fname, contents);
    return;
  }

  // The presence of the PID should make this nearly impossible.
  xfailure("hit loop limit in TemporaryFile");
}


CLOSE_NAMESPACE(smbase)


// EOF
