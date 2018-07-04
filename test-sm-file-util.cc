// test-sm-file-util.cc
// Tests for 'sm-file-util' module.

#include "sm-file-util.h"              // module to test

#include "test.h"                      // USUAL_MAIN, PVAL


static void entry()
{
  SMFileUtil sfu;

  PVAL(sfu.windowsPathSemantics());

  PVAL(sfu.normalizePathSeparators("a/b\\c"));

  PVAL(sfu.currentDirectory());

  PVAL(sfu.isDirectorySeparator('x'));
  PVAL(sfu.isDirectorySeparator('/'));
  PVAL(sfu.isDirectorySeparator('\\'));

  PVAL(sfu.isAbsolutePath("/a/b"));
  PVAL(sfu.isAbsolutePath("/"));
  PVAL(sfu.isAbsolutePath("d:/a/b"));
  PVAL(sfu.isAbsolutePath("//server/share/a/b"));
  PVAL(sfu.isAbsolutePath("\\a\\b"));
  PVAL(sfu.isAbsolutePath("a/b"));
  PVAL(sfu.isAbsolutePath("b"));
  PVAL(sfu.isAbsolutePath("."));
  PVAL(sfu.isAbsolutePath("./a"));

  PVAL(sfu.getAbsolutePath("a"));
  PVAL(sfu.getAbsolutePath("/a"));
  PVAL(sfu.getAbsolutePath("d:/a/b"));
}

USUAL_MAIN

// EOF
