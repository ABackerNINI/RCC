#include "compiler_support.h"
#include "debug_fmt.h"
#include "fmt.h"
#include "paths.h"
#include "utils.h"
#include <algorithm>
#include <iostream>

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
            while (p != std::string::npos) {
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
    std::string includes = "";
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
    std::string temp = template_filename.read_file();

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
        const auto tty_red_bold = TTY_TS(red_bold, stderr);
        if (num_replaced < replaces.size()) {
            print(stderr, tty_red_bold, "ERROR: Template file is missing some placeholders");
        } else /* if (num_replaced > replaces.size()) */ {
            print(stderr, tty_red_bold, "\nERROR: Template file has extra placeholders");
        }
        print(stderr, "\nrcc expects the following placeholders:\n");
        for (const auto &pair : replaces) {
            print(stderr, "  {}\n", pair.first);
        }
        print(stderr, "Please check your template file and try again.\n\n");
        exit(EXIT_FAILURE);
    }

    return temp;
}

std::string linux_gcc::get_compile_command(const std::vector<Path> &sources, const Path &bin_path) const {
    const std::string &cxxflags = settings.get_std_cxxflags_as_string();
    const std::string &additional_flags = settings.get_additional_flags_as_string();

    const Paths &paths = Paths::get_instance();

    std::string compile_cmd = "g++";
    if (!cxxflags.empty()) {
        compile_cmd += " " + cxxflags;
    }

    if (settings.has_included_stdcpp()) {
        compile_cmd += " -DINCLUDE_BITS_STDCPP_H";
    }

    if (!settings.get_additional_includes().empty()) {
        compile_cmd += " -I.";
    }
    compile_cmd += " -I" + paths.get_sub_templates_dir().quote_if_needed();

    compile_cmd += " -include " + paths.get_template_header_path().quote_if_needed();

    compile_cmd += " -o " + bin_path.quote_if_needed();
    for (const auto &source : sources) {
        compile_cmd += " " + source.quote_if_needed();
    }
    if (!additional_flags.empty()) {
        compile_cmd += " " + additional_flags;
    }
    return compile_cmd;
}

std::string linux_clang::get_compile_command(const std::vector<Path> &sources, const Path &bin_path) const {
    const std::string &cxxflags = settings.get_std_cxxflags_as_string();
    const std::string &additional_flags = settings.get_additional_flags_as_string();

    const Paths &paths = Paths::get_instance();

    auto &pch_path = paths.get_template_pch_path();

    std::string compile_cmd = "clang++";
    if (!cxxflags.empty()) {
        compile_cmd += " " + cxxflags;
    }

    if (!settings.get_additional_includes().empty()) {
        compile_cmd += " -I.";
    }
    compile_cmd += " -I" + paths.get_sub_templates_dir().quote_if_needed();

    // Test if the generated PCH is compatible with the given flags.
    // Note: not like g++, clang++ treats PCH mismatch as an error. So we need to test it.
    if (test_pch(settings.get_std(), settings.get_cxxflags(), settings.get_additional_flags())) {
        compile_cmd += " -include-pch " + pch_path.quote_if_needed();
    }

    compile_cmd += " -o " + bin_path.quote_if_needed();
    for (const auto &source : sources) {
        compile_cmd += " " + source.quote_if_needed();
    }
    if (!additional_flags.empty()) {
        compile_cmd += " " + additional_flags;
    }
    return compile_cmd;
}

std::vector<std::string> linux_clang::filter_pch_flags(const std::vector<std::string> &flags) const {
    // These flags have no effect on PCH generation, so we can safely remove them.
    static const std::vector<std::string> simple_flags = {"-c",
                                                          "-o",
                                                          "-S",
                                                          "-pedantic",
                                                          "-w",
                                                          "-shared",
                                                          "-static",
                                                          "-pthread",
                                                          "-pg",
                                                          "-fprofile-arcs",
                                                          "-ftest-coverage",
                                                          "-E",
                                                          "-P",
                                                          "-C",
                                                          "-v",
                                                          "-###",
                                                          "--help",
                                                          "-x",
                                                          "-save-temps"};

    std::vector<std::string> filtered_flags;
    for (const auto &flag : flags) {
        if (std::find(simple_flags.begin(), simple_flags.end(), flag) != simple_flags.end()) {
            continue;
        }

        if (starts_with(flag, "-W") || starts_with(flag, "-L") || starts_with(flag, "-l") || starts_with(flag, "-M")) {
            continue;
        }

        filtered_flags.push_back(flag);
    }

    return filtered_flags;
}

bool linux_clang::get_test_pch_from_cache(const std::string &std,
                                          const std::vector<std::string> &cxxflags,
                                          const std::vector<std::string> &additional_flags,
                                          bool &result) const {
    // * NOTE: we can't sort the flags, consider this case: -D FOO -D BAR

    result = false;

    const Paths &paths = Paths::get_instance();

    const std::string cxxflags_str = vector_to_string(cxxflags, " ");
    const std::string additional_flags_str = vector_to_string(additional_flags, " ");

    const std::string to_hash = std + cxxflags_str + additional_flags_str;

    std::string out_name = u64_to_string_base64x(fnv1a_64_hash_string(to_hash));

    Path outpath = paths.get_sub_clang_pch_test_cache_dir() / out_name;

    // Check if the file exists
    if (!outpath.exists()) {
        return false;
    }

    try {
        std::ifstream file(outpath.string());
        if (!file.is_open()) {
            return false;
        }

        std::string std_read, cxxflags_read, additional_flags_read, result_read;
        if (!std::getline(file, std_read) || !std::getline(file, cxxflags_read) ||
            !std::getline(file, additional_flags_read) || !std::getline(file, result_read)) {
            return false;
        }

        file.close();

        if (std_read != std || cxxflags_read != cxxflags_str || additional_flags_read != additional_flags_str) {
            gpdebug("Clang test PCH cache file content mismatch");
            return false;
        }

        result = (result_read == "true");
        return true;
    } catch (const std::exception &e) {
        gpwarning(TTY_TS(fg(terminal_color::red), stderr), "Failed to read clang test PCH cache file: {}\n", e.what());
        return false;
    }
}

void linux_clang::save_test_pch_to_cache(const std::string &std,
                                         const std::vector<std::string> &cxxflags,
                                         const std::vector<std::string> &additional_flags,
                                         bool result) const {
    const Paths &paths = Paths::get_instance();

    const std::string cxxflags_str = vector_to_string(cxxflags, " ");
    const std::string additional_flags_str = vector_to_string(additional_flags, " ");

    const std::string to_hash = std + cxxflags_str + additional_flags_str;

    std::string out_name = u64_to_string_base64x(fnv1a_64_hash_string(to_hash));

    Path outpath = paths.get_sub_clang_pch_test_cache_dir() / out_name;

    try {
        std::ofstream out_file(outpath.string());
        out_file << std << "\n"
                 << cxxflags_str << "\n"
                 << additional_flags_str << "\n"
                 << (result ? "true" : "false") << "\n";
        out_file.close();
    } catch (const std::exception &e) {
        gpwarning(TTY_TS(fg(terminal_color::red), stderr), "Failed to write clang pch test file: {}\n", e.what());
    }
}

bool linux_clang::test_pch(const std::string &std,
                           const std::vector<std::string> &cxxflags,
                           const std::vector<std::string> &additional_flags) const {
    auto time_begin = now();

    auto filtered_cxxflags = filter_pch_flags(cxxflags);
    auto filtered_additional_flags = filter_pch_flags(additional_flags);

    // TODO: return true if the flags are defaults

    // If already in cache, return the cached result.
    bool result;
    if (get_test_pch_from_cache(std, filtered_cxxflags, filtered_additional_flags, result)) {
        gpinfo("clang pch test cached ({:.2f}ms)\n", duration_ms(time_begin));
        return result;
    }

    const Paths &paths = Paths::get_instance();

    auto &gch_path = paths.get_template_pch_path();

    // -E: preprocess only, -P: remove line markers
    const std::string test_cmd = "clang++ " + std + " " + vector_to_string(filtered_cxxflags, " ", "", true) +
                                 "-x c++ -E -P -include-pch " + gch_path.quote_if_needed() + " /dev/null " +
                                 vector_to_string(filtered_additional_flags, " ", "", true) + "> /dev/null 2>&1";

    result = system(test_cmd.c_str()) == 0;

    const auto tty_ts = TTY_TS(result ? fg(terminal_color::green) : fg(terminal_color::red), stderr);
    gpdebug("PCH test result: {}\n", styled(result ? "true" : "false", tty_ts));
    gpmsgdump("TEST CMD: {}\n", test_cmd);

    gpinfo("PCH test took {:.2f} ms\n", duration_ms(time_begin, now()));

    save_test_pch_to_cache(std, filtered_cxxflags, filtered_additional_flags, result);

    return result;
}

std::unique_ptr<compiler_support> create_compiler_support(const std::string &compiler_name, const Settings &settings) {
    // Note: std::make_unique is not available in C++11
    if (compiler_name == "g++") {
        return std::unique_ptr<compiler_support>(new linux_gcc(settings));
    } else if (compiler_name == "clang++") {
        return std::unique_ptr<compiler_support>(new linux_clang(settings));
    } else {
        std::cerr << "Unsupported compiler: " + compiler_name + "\n";
        exit(EXIT_FAILURE);
    }
}

} // namespace rcc
