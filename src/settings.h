#ifndef __SETTINGS_H__
#define __SETTINGS_H__

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
    // Parse the command line arguments and initialize the settings.
    int parse_argv(int argc, char **argv);

    const std::string get_compiler() const { return compiler; }
    const std::string get_std() const { return std; }
    bool get_clean_cache_flag() const { return clean_cache; }
    const std::vector<std::string> &get_additional_flags() const { return additional_flags; }
    const std::vector<std::string> &get_additional_includes() const { return additional_includes; }
    const std::vector<std::string> &get_above_main() const { return above_main; }
    const std::vector<std::string> &get_functions() const { return functions; }
    const std::vector<std::string> &get_codes() const { return codes; }
    const std::vector<std::string> &get_cxxflags() const { return cxxflags; }
    const std::vector<std::string> &get_additional_sources() const { return additional_sources; }

    std::string get_cxxflags_as_string() const;
    std::string get_additional_flags_as_string() const { return vector_to_string(additional_flags); }
    std::string get_cli_args_as_string() const;
    std::string get_additional_includes_as_string() const { return vector_to_string(additional_includes); }
    std::string get_above_main_as_string() const { return vector_to_string(above_main, "\n"); }
    std::string get_functions_as_string() const { return vector_to_string(functions, "\n"); }
    std::string get_codes_as_string() const { return merge_codes(codes); }
    std::string get_additional_sources_as_string() const { return vector_to_string(additional_sources); }

    // Check if at least one code snippet is present.
    bool has_code() const { return !codes.empty(); }

    // Check if the `bits/stdc++.h` header is included.
    // It is true if the "--include-all" option is used or if the `bits/stdc++.h` header is
    // explicitly included by the "--include" option.
    bool has_included_stdcpp() const { return included_stdcpp; }

    // Print the settings to standard error for debugging purposes.
    void debug_print() const;

  private:
    // Concatenate a vector of strings into one string with a separator.
    static std::string vector_to_string(const std::vector<std::string> &vec, const std::string &sep = " ");

    // Merge multiple code snippets into one string.
    static std::string merge_codes(const std::vector<std::string> &codes);

  private:
    std::string compiler = RCC_COMPILER; // the compiler to use
    std::string std = RCC_CPP_STD; // the c++ standard to use, relates to "-std"
    // the c++ flags to use, default to a set of common flags.
    std::vector<std::string> cxxflags = {"-Wall",
                                         "-Wextra",
                                         "-Wno-unused-variable",
                                         "-Wno-unused-parameter",
                                         "-Wno-unused-function"};
    // additional flags to pass to the compiler, at the end of the compile command, the libraries to link against have
    // to be specified after source files that use them. Relates to anything that starts with "-l", or "-pthread", or
    // "-math".
    std::vector<std::string> additional_flags;
    std::vector<std::string> additional_includes; // additional include headers, relates to "--include" and
                                                  // "--include-all"
    std::vector<std::string> above_main; // code to put above the main function, relates to "--put-above-main"
    std::vector<std::string> functions; // functions to declare before the main function, relates to "--function"
    std::vector<std::string> codes; // the command line code snippets
    std::vector<std::string> additional_sources; // additional source files to compile with, relates to "--compile-with"

    bool clean_cache = false; // whether to clean the cache, relates to "--clean-cache"

    // the start of the arguments that are going to pass to the program, anything after "--".
    //* These arguments will not be parsed by CLI11, and will be passed to the program as is.
    char **args_start = NULL;
    int args_count = 0; // the number of arguments

    bool included_stdcpp = false; // whether the `bits/stdc++.h` has been included, relates to "--include-all"
                                  // and"--include"
};

} // namespace rcc

#endif // __SETTINGS_H__
