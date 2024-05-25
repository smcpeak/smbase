// d2vector-test.c
// Tests for d2vector.

#include "d2vector.h"                  // module under test

#include "dummy-printf.h"              // dummy_printf

#include <stdio.h>                     // printf


static int verbose = 0;
#define printf (verbose? printf : dummy_printf)

// TODO: These do not respond to `verbose`.
#define printD2Point(p) ((void)(p))
#define printD2Line(p) ((void)(p))


static void runIntersect(double px, double py, double vx, double vy,
                         double qx, double qy, double wx, double wy)
{
  D2Line L1, L2;
  double t;

  printf("computing intersection:\n");

  L1.origin.x = px;
  L1.origin.y = py;
  L1.vector.x = vx;
  L1.vector.y = vy;
  printf("  L1: "); printD2Line(&L1); printf("\n");

  L2.origin.x = qx;
  L2.origin.y = qy;
  L2.vector.x = wx;
  L2.vector.y = wy;
  printf("  L2: "); printD2Line(&L2); printf("\n");

  t = intersectD2Lines(&L1, &L2);
  if (isSpecial(t)) {
    printf("  these lines are parallel\n");
  }
  else {
    D2Point p;

    printf("  t is %g\n", t);

    // compute it as a point
    pointOnD2Line(&p, &L2, t);
    printf("  intersection is (%g,%g)\n", p.x, p.y);
  }
}


static void rotTest()
{
  double pi = 3.14159265358979323846;

  D2Vector v;
  int i;

  printf("\nrot test\n");

  v.x = 2;
  v.y = 1;
  printD2Point(&v);
  rotD2Vector90(&v);
  printD2Point(&v);
  rotD2Vector180(&v);
  printD2Point(&v);
  rotD2Vector270(&v);
  printD2Point(&v);
  printf("\n");

  v.x = 1;
  v.y = 0;
  printD2Point(&v);
  for (i=0; i<12; i++) {
    D2Vector tmp;
    if (i % 3 == 0) {
      printf("\n");
    }
    rotD2VectorAngle(&tmp, &v, 30.0 * pi / 180.0);   // 30 degrees
    v = tmp;
    printD2Point(&v);
  }
  printf("\n");
}


// Called from unit-tests.cc.
void test_d2vector()
{
  // from (1,0) pointing up, intersected with
  // from (0,1) pointing right
  runIntersect(1,0, 0,1,  0,1, 1,0);
  runIntersect(0,1, 1,0,  1,0, 0,1);

  // vertical and diagonal
  runIntersect(1,0, 0,1,  0,0, 1,1);

  // vertical and diagonal
  runIntersect(1,0, 0,1,  0,0, 1,2);

  // vertical and diagonal
  runIntersect(2,0, 0,1,  0,0, 1,2);

  // parallel vertical
  runIntersect(1,0, 0,1,  0,0, 0,1);

  // parallel horizontal
  runIntersect(0,1, 1,0,  0,0, 1,0);

  // horizontal and diagonal
  runIntersect(0,1, 1,0,  0,10, 1,-0.1);

  // both diagonal, not parallel
  runIntersect(1,0, 1,2,  0,1, 2,1);

  // diagonal and parallel (but the vector isn't identical)
  runIntersect(1,0, 1,2,  0,1, 2,4);

  rotTest();
}


// EOF
