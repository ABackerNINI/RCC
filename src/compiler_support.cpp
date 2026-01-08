#include "pch.h"

#include "compiler_support.h"
#include "utils.h"

namespace rcc {

class safe_replacer {
  private:
    struct pos {
        size_t p;
        size_t index;
    };

  public:
    static size_t replace(std::string &str, const std::vector<std::pair<std::string, std::string>> &replaces) {
        // Find all occurrences of the strings to be replaced and store their positions along with their indices in the
        // replaces vector
        std::vector<pos> positions;
        for (size_t i = 0; i < replaces.size(); i++) {
            size_t p = str.find(replaces[i].first);
            while (p != string::npos) {
                positions.push_back({p, i});
                p = str.find(replaces[i].first, p + replaces[i].first.size());
            }
        }

        // If no occurrences were found, return 0
        if (positions.empty()) {
            return 0;
        }

        // Sort positions by position in reverse order so that replacements do not interfere with each other
        std::sort(positions.begin(), positions.end(), [](const pos &a, const pos &b) { return a.p > b.p; });

        // Replace all occurrences in reverse order
        for (auto &p : positions) {
            str.replace(p.p, replaces[p.index].first.size(), replaces[p.index].second);
        }

        // Return the number of replacements made
        return positions.size();
    }
};

std::string compiler_support::gen_additional_includes(const std::vector<std::string> &additional_includes) const {
    string includes = "";
    for (auto &inc : additional_includes) {
        if (inc == "bits/stdc++.h") {
            //* g++ and clang++ uses different strategies for PCH.
            //* g++ can only include one PCH for one translation unit. Since we already include `rcc_template.hpp`
            //* as PCH, so we cannot include `rcc_bits_stdc++.hpp` as PCH here.
            //* g++ can match the correct PCH inside one directory. For example, for the `rcc_template.hpp`, we
            //* can generate two versions of PCHs inside the `rcc_template.hpp.pch` directory using different compiler
            //* flags, one defines the macro `INCLUDE_BITS_STDCPP_H` so that the `bits/stdc++.h` can be included, and
            //* the other does not define the macro. When compiling a translation unit, g++ will match the correct PCH
            //* inside the `rcc_template.hpp.pch` directory based on the compiler flags.
            //* clang++ seems to have a problem with including multiple PCHs, so we use a similar strategy as g++
            // includes += "#include \"rcc_bits_stdc++.hpp\"\n";
        } else {
            if (Path(inc).exists()) {
                includes += "#include \"" + inc + "\"\n";
            } else {
                includes += "#include <" + inc + ">\n";
            }
        }
    }
    if (includes.length() > 0) {
        includes.pop_back(); // remove last newline character
    }
    return includes;
}

std::string compiler_support::gen_code(const Path &template_filename,
                                       const std::vector<std::string> &includes,
                                       const std::vector<std::string> &above_main,
                                       const std::vector<std::string> &functions,
                                       const std::string &commandline_code,
                                       const std::string &identifier) const {
    string temp = template_filename.read_file();

    // The template file should be checked during installation
    // so do not check it here

    std::vector<std::pair<std::string, std::string>> replaces;
    replaces.push_back({"$rcc-inc", "User includes\n" + gen_additional_includes(includes)});
    replaces.push_back({"$rcc-above-main", "User above main\n" + vector_to_string(above_main, "\n")});
    replaces.push_back({"$rcc-func", "User functions\n" + vector_to_string(functions, "\n")});
    replaces.push_back({"$rcc-code", "User codes\n    " + commandline_code});
    replaces.push_back({"$rcc-id", "ID: " + identifier});

    size_t num_replaced = safe_replacer::replace(temp, replaces);

    if (num_replaced != replaces.size()) {
        if (num_replaced < replaces.size()) {
            print(fmt::bg(fmt::color::red), "WARNING: Template file is missing some placeholders");
        } else if (num_replaced > replaces.size()) {
            print(fmt::bg(fmt::color::red), "\nWARNING: Template file has extra placeholders");
        }
        print("\nrcc expects the following placeholders:\n");
        for (const auto &pair : replaces) {
            print("  {}\n", pair.first);
        }
        print("Please check your template file and try again.\n\n");
    }

    return temp;
}

std::string linux_gcc::get_compile_command(const std::vector<Path> &sources,
                                           const Path &bin_path,
                                           const std::string &cxxflags,
                                           const std::string &additional_flags) const {
    Paths &paths = Paths::get_instance();

    string compile_cmd = "g++";
    if (cxxflags != "") {
        compile_cmd += " " + cxxflags;
    }

    if (!settings.get_additional_includes().empty()) {
        compile_cmd += " -I.";
    }
    compile_cmd += " -I" + paths.get_sub_templates_dir().get_path();

    // g++ will detect PCH files automatically, so we don't need to add them here
    if (settings.has_included_stdcpp()) {
        compile_cmd += " -DINCLUDE_BITS_STDCPP_H";
    }

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

    string compile_cmd = "clang++";
    if (cxxflags != "") {
        compile_cmd += " " + cxxflags;
    }

    if (!settings.get_additional_includes().empty()) {
        compile_cmd += " -I.";
    }
    compile_cmd += " -I" + paths.get_sub_templates_dir().get_path();

    // clang++ needs to specify the .pch file explicitly
    if (settings.has_included_stdcpp()) {
        pch_path.join("rcc_template.hpp.stdc++.pch");
        compile_cmd += " -DINCLUDE_BITS_STDCPP_H";
        compile_cmd += " -include-pch " + pch_path.get_path();
    } else {
        pch_path.join("rcc_template.hpp.pch");
        compile_cmd += " -include-pch " + pch_path.get_path();
    }

    compile_cmd += " -o " + bin_path.get_path();
    for (const auto &source : sources) {
        compile_cmd += " " + source.get_path();
    }
    if (additional_flags != "") {
        compile_cmd += " " + additional_flags;
    }
    return compile_cmd;
}

compiler_support *new_compiler_support(const std::string &compiler_name, const Settings &settings) {
    compiler_support *cs = NULL;
    if (compiler_name == "g++") {
        cs = new linux_gcc(settings);
    } else if (compiler_name == "clang++") {
        cs = new linux_clang(settings);
    } else {
        cerr << "Unsupported compiler: " + compiler_name + "\n";
        exit(EXIT_FAILURE);
    }
    return cs;
}

} // namespace rcc
