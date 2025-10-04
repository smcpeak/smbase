// check-fts.cc
// Compile the output of `file-to-strlit.py` and check that it decodes
// to the original file.

// The `arr` array must be declared via a `-include` command line
// argument.

#include <algorithm>                   // std::min
#include <cstddef>                     // std::size_t
#include <fstream>                     // std::ifstream
#include <iostream>                    // std::cout
#include <sstream>                     // std::ostringstream

int main(int argc, char **argv)
{
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <origfile>\n";
    return 2;
  }
  char const *fname = argv[1];

  std::ifstream in(fname, std::ios::binary);
  if (!in) {
    std::cout << "Failed to open " << fname << "\n";
    return 2;
  }

  // Read the file to get the expected contents of `arr`.
  std::ostringstream oss;
  oss << in.rdbuf();
  std::string expect = oss.str();

  // The `arr` array is declared with one extra byte.
  std::string actual(arr, sizeof(arr)-1);

  if (actual != expect) {
    std::cout << "Actual size is " << actual.size() << ".\n";
    std::cout << "Expect size is " << expect.size() << ".\n";
    for (std::size_t i = 0;
         i < std::min(actual.size(), expect.size());
         ++i) {
      if (actual.at(i) != expect.at(i)) {
        std::cout << "At index " << i
                  << ", actual byte is " << (int)actual.at(i)
                  << ", expect byte is " << (int)expect.at(i) << "\n";
      }
    }
    std::cout << "Contents differ.\n";
    return 1;
  }

  return 0;
}

// EOF
