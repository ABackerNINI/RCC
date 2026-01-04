#include "compiler_support.h"

#include <iostream>

using namespace std;

namespace rcc {

std::string linux_gcc::get_compile_command(const std::vector<Path> &sources,
                                           const Path &bin_path,
                                           const std::string &cxxflags,
                                           const std::string &additional_flags) const {
    Paths &paths = Paths::get_instance();

    string compile_cmd = "g++";
    if (cxxflags != "") {
        compile_cmd += " " + cxxflags;
    }

    // g++ will detect PCH files automatically, so we don't need to add them here

    compile_cmd += " -I" + paths.get_sub_templates_dir().get_path();
    compile_cmd += " -o " + bin_path.get_path();
    for (const auto &source : sources) {
        compile_cmd += " " + source.get_path();
    }
    if (additional_flags != "") {
        compile_cmd += " " + additional_flags;
    }
    return compile_cmd;
}

std::string linux_clang::get_compile_command(const std::vector<Path> &sources,
                                             const Path &bin_path,
                                             const std::string &cxxflags,
                                             const std::string &additional_flags) const {
    Paths &paths = Paths::get_instance();

    Path pch_path = paths.get_sub_templates_dir();
    pch_path.join("rcc_template.hpp.pch");

    string compile_cmd = "clang++";
    if (cxxflags != "") {
        compile_cmd += " " + cxxflags;
    }

    // clang++ needs to specify the pch file explicitly
    compile_cmd += " -Xclang -include-pch -Xclang " + pch_path.get_path();

    compile_cmd += " -I" + paths.get_sub_templates_dir().get_path();
    compile_cmd += " -o " + bin_path.get_path();
    for (const auto &source : sources) {
        compile_cmd += " " + source.get_path();
    }
    if (additional_flags != "") {
        compile_cmd += " " + additional_flags;
    }
    return compile_cmd;
}

compiler_support *new_compiler_support(const std::string &compiler_name) {
    compiler_support *cs = NULL;
    if (compiler_name == "g++") {
        cs = new linux_gcc();
    } else if (compiler_name == "clang++") {
        cs = new linux_clang();
    } else {
        cerr << "Unsupported compiler: " + compiler_name + "\n";
        exit(EXIT_FAILURE);
    }
    return cs;
}

} // namespace rcc
