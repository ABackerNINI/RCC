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

    const std::string get_compiler() const { return compiler; }
    const std::string get_std() const { return std; }
    bool get_clean_cache_flag() const { return clean_cache; }
    const std::vector<std::string> &get_additional_flags() const { return additional_flags; }
    const std::vector<std::string> &get_additional_includes() const { return additional_includes; }
    const std::vector<std::string> &get_functions() const { return functions; }
    const std::vector<std::string> &get_codes() const { return codes; }
    const std::vector<std::string> &get_cxxflags() const { return cxxflags; }

    std::string get_cxxflags_as_string() const;
    std::string get_additional_flags_as_string() const { return vector_to_string(additional_flags); }
    std::string get_cli_args_as_string() const;
    std::string get_additional_includes_as_string() const { return vector_to_string(additional_includes); }
    std::string get_functions_as_string() const { return vector_to_string(functions); }
    std::string get_codes_as_string() const { return merge_codes(codes); }

    bool has_code() const { return !codes.empty(); }
    bool has_included_stdcpp() const { return included_stdcpp; }

    void debug_print(std::ostream &os = std::cerr) const;

  private:
    static std::string vector_to_string(const std::vector<std::string> &vec, const std::string &sep = " ");
    static std::string merge_codes(const std::vector<std::string> &codes);

  private:
    std::string compiler = RCC_COMPILER;
    std::string std = RCC_CPP_STD;
    bool clean_cache = false;
    std::vector<std::string> additional_flags;
    std::vector<std::string> additional_includes;
    std::vector<std::string> functions;
    std::vector<std::string> codes; // the command line code snippets
    std::vector<std::string> cxxflags = {
        "-Wall", "-Wextra", "-Wno-unused-variable", "-Wno-unused-parameter", "-Wno-unused-function"};

    char **args_start = NULL;
    int args_count = 0;

    bool included_stdcpp = false;
};

} // namespace rcc

#endif // __SETTINGS_H__
