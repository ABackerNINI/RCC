#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <iostream>
#include <string>
#include <vector>

#ifndef RCC_COMPILER
// Use this compiler to compile the code.
#define RCC_COMPILER "g++"
#endif

#ifndef RCC_CPP_STD
// Use this standard to compile the code, at least c++11.
#define RCC_CPP_STD "c++17"
#endif

namespace rcc {

class Settings {
  public:
int parse_argv(int argc, char **argv);

    std::string gen_cxxflags() const;
    std::string gen_additional_flags() const;
    std::string gen_command_line_code() const;
    std::string gen_command_line_args() const;

    bool has_code() const;

    void debug_print(std::ostream &os = std::cerr) const;

  private:
    std::vector<std::string> cxxflags = {
        "-Wall", "-Wextra", "-Wno-unused-variable", "-Wno-unused-parameter", "-Wno-unused-function"};
    std::vector<std::string> additional_flags;

    char **code_start = NULL;
    int code_count = 0;
    char **args_start = NULL;
    int args_count = 0;

  public:
    std::string compiler = RCC_COMPILER;
    std::string std = RCC_CPP_STD;
    bool clean_cache = false;
};


} // namespace rcc

#endif // __SETTINGS_H__
