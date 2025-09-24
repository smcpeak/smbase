// gdvalue-srcloc-mgr-test.cc
// Tests for `gdvalue-srcloc-mgr` module.

#include "smbase/gdvalue-srcloc-mgr.h" // module under test

#include "smbase/exc.h"                // EXN_CONTEXT_EXPR
#include "smbase/gdvalue.h"            // gdv::toGDValue for EXPECT_EQ_GDVSER
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-random.h"          // smbase::RandomChoice
#include "smbase/sm-test.h"            // EXPECT_EQ, EXPECT_EQ_GDVSER, TEST_FUNC_EXPRS, envRandomizedTestIters

#include <utility>                     // std::pair

using namespace gdv;
using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


using FileIndex =
  GDValueSourceLocationManager::FileIndex;
using LineNumber =
  GDValueSourceLocationManager::LineNumber;
using EncodedFileAndLine =
  GDValueSourceLocationManager::EncodedFileAndLine;
using FileLinePair =
  std::pair<FileIndex, LineNumber>;


void checkDecode(
  GDValueSourceLocationManager const &mgr,
  EncodedFileAndLine efal,
  FileIndex fi,
  LineNumber line)
{
  TEST_FUNC_EXPRS(efal, fi, line);

  EXPECT_EQ_GDVSER(mgr.decodeFileAndLine(efal), FileLinePair(fi, line));
}


void test_basics()
{
  TEST_FUNC();

  // Empty manager.
  GDValueSourceLocationManager mgr;

  mgr.selfCheck();
  EXPECT_FALSE(mgr.validFileIndex(0));
  EXPECT_FALSE(mgr.validEncodedFileAndLine(0));

  // One file, but no encoded space yet.
  FileIndex fi1 = mgr.fileIndexForName("file1");

  mgr.selfCheck();
  EXPECT_EQ(fi1, 0);
  EXPECT_TRUE(mgr.validFileIndex(fi1));
  EXPECT_EQ(mgr.fileNameForIndex(fi1), "file1");
  EXPECT_FALSE(mgr.validEncodedFileAndLine(0));

  // Encode line 10.
  EncodedFileAndLine fi1_10 = mgr.encodeFileAndLine(fi1, 10);

  mgr.selfCheck();
  EXPECT_EQ(fi1_10, 10);     // Goes at the start since otherwise empty.
  EXPECT_TRUE(mgr.validEncodedFileAndLine(0));
  EXPECT_TRUE(mgr.validEncodedFileAndLine(fi1_10));
  checkDecode(mgr, fi1_10, fi1, 10);

  // Encode line 20.
  EncodedFileAndLine fi1_20 = mgr.encodeFileAndLine(fi1, 20);

  mgr.selfCheck();
  EXPECT_EQ(fi1_20, 20);     // Contiguous with previous.
  EXPECT_TRUE(mgr.validEncodedFileAndLine(0));
  EXPECT_TRUE(mgr.validEncodedFileAndLine(fi1_10));
  EXPECT_TRUE(mgr.validEncodedFileAndLine(fi1_20));
  checkDecode(mgr, fi1_10, fi1, 10);
  checkDecode(mgr, fi1_20, fi1, 20);

  // Second file.
  FileIndex fi2 = mgr.fileIndexForName("file2");

  mgr.selfCheck();
  EXPECT_EQ(fi2, 1);
  EXPECT_TRUE(mgr.validFileIndex(fi1));
  EXPECT_TRUE(mgr.validFileIndex(fi2));
  EXPECT_EQ(mgr.fileNameForIndex(fi1), "file1");
  EXPECT_EQ(mgr.fileNameForIndex(fi2), "file2");

  // Encode line 1 in the second file.
  EncodedFileAndLine fi2_1 = mgr.encodeFileAndLine(fi2, 1);

  mgr.selfCheck();
  EXPECT_EQ(fi2_1, 257);     // Initial allocation is 256 for `fi1`.
  EXPECT_TRUE(mgr.validEncodedFileAndLine(fi1_10));
  EXPECT_TRUE(mgr.validEncodedFileAndLine(fi1_20));
  EXPECT_TRUE(mgr.validEncodedFileAndLine(fi2_1));
  checkDecode(mgr, fi1_10, fi1, 10);
  checkDecode(mgr, fi1_20, fi1, 20);
  checkDecode(mgr, fi2_1, fi2, 1);

  // Allocate more space for the first file.
  EncodedFileAndLine fi1_255 = mgr.encodeFileAndLine(fi1, 255);

  mgr.selfCheck();
  EXPECT_EQ(fi1_255, 255);   // Just inside initial allocation.
  checkDecode(mgr, fi1_255, fi1, 255);

  // Even more.
  EncodedFileAndLine fi1_256 = mgr.encodeFileAndLine(fi1, 256);

  mgr.selfCheck();
  EXPECT_EQ(fi1_256, 512);   // Added a new fragment.
  checkDecode(mgr, fi1_256, fi1, 256);

  // Adding the same files again yields the same indices.
  EXPECT_EQ(mgr.fileIndexForName("file1"), fi1);
  EXPECT_EQ(mgr.fileIndexForName("file2"), fi2);

  mgr.selfCheck();
}


// This class keeps track of data sent to and from the source loc
// manager, and then checks for correspondence in `selfCheck`.
class ManagerMonitor {
public:      // types
  // An encoded location.
  struct Encoding {
    FileIndex m_fileIndex;
    LineNumber m_lineNumber;
    EncodedFileAndLine m_encoding;

    FileLinePair locPair() const
    {
      return {m_fileIndex, m_lineNumber};
    }
  };

public:      // data
  // System under test.
  GDValueSourceLocationManager m_mgr;

  // File names allocated, indexed by FileIndex.
  std::vector<std::string> m_fnames;

  // Current file lengths.
  std::vector<int> m_lengths;

  // Encodings we have created.
  std::vector<Encoding> m_encodings;

public:      // methods
  ManagerMonitor()
  :
    m_mgr(),
    m_fnames(),
    m_lengths(),
    m_encodings()
  {}

  void selfCheck() const
  {
    m_mgr.selfCheck();

    // Verify the file name mapping.
    for (FileIndex fi=0; fi < numFiles(); ++fi) {
      EXPECT_TRUE(m_mgr.validFileIndex(fi));
      EXPECT_EQ(m_mgr.fileNameForIndex(fi), m_fnames.at(fi));
    }
    EXPECT_FALSE(m_mgr.validFileIndex(numFiles()+1));

    // Verify all encoded locations.
    for (Encoding const &enc : m_encodings) {
      EXPECT_TRUE(m_mgr.validEncodedFileAndLine(enc.m_encoding));
      EXPECT_EQ_GDVSER(m_mgr.decodeFileAndLine(enc.m_encoding),
                       enc.locPair());
      EXPECT_EQ(m_mgr.getEncodedFileAndLine(
                  enc.m_fileIndex, enc.m_lineNumber),
                enc.m_encoding);
    }
  }

  FileIndex numFiles() const
  {
    return m_fnames.size();
  }

  void allocateFile()
  {
    std::string fname = stringb("file" << numFiles());
    FileIndex fi = m_mgr.fileIndexForName(fname);
    xassert(fi == numFiles());
    m_fnames.push_back(fname);
    m_lengths.push_back(0);
  }

  void extendRandomFile()
  {
    if (m_fnames.empty()) {
      return;
    }

    FileIndex fi = sm_random(numFiles());
    int extensionAmount = (0x80 << sm_random(6));

    LineNumber ln = (m_lengths.at(fi) += extensionAmount);
    EncodedFileAndLine enc = m_mgr.encodeFileAndLine(fi, ln);
    m_encodings.push_back({fi, ln, enc});
  }
};


void test_randomInsertions()
{
  TEST_FUNC();

  int const OUTER_ITERS =
    envRandomizedTestIters(10, "GSMR_OUTER_ITERS", 2);
  int const INNER_ITERS =
    envRandomizedTestIters(100, "GSMR_INNER_ITERS", 2);

  smbase_loopi(OUTER_ITERS) {
    EXN_CONTEXT_EXPR(i);

    ManagerMonitor monitor;
    monitor.selfCheck();

    monitor.allocateFile();
    monitor.selfCheck();

    smbase_loopj(INNER_ITERS) {
      EXN_CONTEXT_EXPR(j);

      RandomChoice choice(100);

      if (choice.check(10)) {
        monitor.allocateFile();
      }

      else if (choice.check(89)) {
        monitor.extendRandomFile();
      }

      else {
        // The check is pretty expensive, so only do it occasionally on
        // longer runs so I can make larger structures.
        monitor.selfCheck();
      }
    }

    monitor.selfCheck();
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_srcloc_mgr()
{
  test_basics();
  test_randomInsertions();
}


// EOF
