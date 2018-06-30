// testcout.cc
// very simple test of the C++ compiler and runtime

#include <iostream>          // cout

class Superclass {
public:
  virtual int foo();
};

class Subclass : public Superclass {
public:
  // I depend on being able to use 'override' in smbase, so test
  // that the compiler accepts it now.
  int foo() override;
};

int main()
{
  std::cout << "testcout: works\n";
  return 0;
}
