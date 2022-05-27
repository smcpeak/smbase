// array-test.cc
// Test for for 'array' module.

#include "array.h"                     // module under test

#include <algorithm>                   // std::sort
#include <cstdlib>                     // std::rand
#include <vector>                      // std::vector


// Assert that 'ase' and 'vec' have the same sequence of elements.
template <class T, int n>
static void checkEqual(ArrayStackEmbed<T,n> const &ase,
                       std::vector<T> const &vec)
{
  xassert(ase.size() == vec.size());
  xassert(ase.empty() == vec.empty());

  for (size_t i=0; i < ase.size(); i++) {
    xassert(ase[i] == vec[i]);
  }
}


// Test that std::sort works on ArrayStackEmbed.
static void testEmbedArraySort()
{
  ArrayStackEmbed<int,2> ase;
  std::vector<int> vec;

  // One specific input.
  for (int i : (int[]){3,5,4}) {
    ase.push_back(i);
    vec.push_back(i);
  }
  checkEqual(ase, vec);

  // Sort both.
  std::sort(ase.begin(), ase.end());
  std::sort(vec.begin(), vec.end());
  checkEqual(ase, vec);

  // Clear both.
  ase.clear();
  vec.clear();
  checkEqual(ase, vec);

  // A bunch of random inputs.
  for (int len=0; len < 10; len++) {
    int iterCount = len*5 + 1;
    for (int iter=0; iter < iterCount; iter++) {
      // Also test with a fresh array.
      ArrayStackEmbed<int,2> ase2;

      // And another, with a different size.
      ArrayStackEmbed<int,5> ase3;

      // Populate the arrays.
      for (int i=0; i < len; i++) {
        int num = std::rand() % 20;
        ase.push_back(num);
        ase2.push_back(num);
        ase3.push_back(num);
        vec.push_back(num);
      }
      checkEqual(ase, vec);
      checkEqual(ase2, vec);
      checkEqual(ase3, vec);

      // Sort.
      std::sort(ase.begin(), ase.end());
      std::sort(ase2.begin(), ase2.end());
      std::sort(ase3.begin(), ase3.end());
      std::sort(vec.begin(), vec.end());
      checkEqual(ase, vec);
      checkEqual(ase2, vec);
      checkEqual(ase3, vec);

      // Clear.
      ase.clear();
      ase2.clear();
      ase3.clear();
      vec.clear();
      checkEqual(ase, vec);
      checkEqual(ase2, vec);
      checkEqual(ase3, vec);
    }
  }
}


void test_array()
{
  testEmbedArraySort();
}


// EOF
