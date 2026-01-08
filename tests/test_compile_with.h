#ifndef __TEST_COMPILE_WITH_H__
#define __TEST_COMPILE_WITH_H__

#include <iostream>

namespace rcc {

// A simple function that prints what is passed to it.
void print_string(std::string str);

// A simple class.
class TestClass {
  public:
    void test_method() { std::cout << "Test method called!" << std::endl; }
};

} // namespace rcc

#endif // __TEST_COMPILE_WITH_H__
