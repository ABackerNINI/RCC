#include "settings.h"

#include <cstring>

#include "libs/CLI11.hpp"
#include "libs/rang.hpp"

using namespace std;

namespace rcc {

static int locate_code(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || (strlen(argv[i]) >= 2 && (!isalpha(argv[i][1])) && argv[i][1] != '-')) {
            return i;
        }
    }
    return argc;
}

static int locate_args(char **code_start, int code_count) {
    for (int i = 0; i < code_count; i++) {
        if (strcmp(code_start[i], "--") == 0) {
            return i + 1;
        }
    }
    return code_count;
}

static std::string vector_to_string(const std::vector<std::string> &vec) {
    string result = "";
    for (const auto &item : vec) {
        result += item + " ";
    }
    if (result.size() > 0) {
        result.pop_back();
    }
    return result;
}

// Merge all command line arguments together into a string.
static string merge_args(char **code_start, int n) {
    string s;
    s.reserve(256);
    s += "User code\n    ";
    // If there is only one argument and it doesn't end with ';', wrap it in
    // 'cout << ... << endl;' directly. This is for convenience, e.g. rcc '2+3*5'.
    if (n == 1 && strlen(code_start[0]) > 0 && code_start[0][strlen(code_start[0]) - 1] != ';') {
        s += "cout << " + string(code_start[0]) + " << endl;";
    } else {
        // Otherwise, merge all arguments together without any modification.
        for (int i = 0; i < n; ++i) {
            s.append(code_start[i]);
        }
    }
    return s;
}

int Settings::parse_argv(int argc, char **argv) {
    int code_index = locate_code(argc, argv);

    code_start = &argv[code_index];
    code_count = argc - code_index;
    argc -= code_count;

    int args_index = locate_args(code_start, code_count);
    args_start = &code_start[args_index];
    args_count = code_count - args_index;
    code_count -= args_count + 1;

    CLI::App app{"RCC - Run C/C++ codes in terminal"};
    app.add_flag("--clean_cache", clean_cache, "Clean cached source and binary files");
    app.allow_non_standard_option_names();
    app.allow_extras();
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        std::cout << (e.get_exit_code() == 0 ? rang::fg::blue : rang::fg::red);
        return app.exit(e);
    }
    vector<string> remaining = app.remaining(true);

    for (auto &s : remaining) {
        if (s.substr(0, 4) == "-std") {
            std = s;
        } else if (s.substr(0, 2) == "-l" || s == "-math" || s == "-pthread") {
            additional_flags.push_back(s);
        } else {
            cxxflags.push_back(s);
        }
    }

    return 0;
}

std::string Settings::gen_cxxflags() const {
    //? Should use a C++ standard like "-std=c++11"?
    //* This will be necessary on some lower version compilers. But this will
    //* prevent us using a higher default standard.
    //* The current solution is test C++ standard before making rcc, then use
    //* the highest possible standard to compile the rcc and template header as
    //* well as use it here. See RCC_CPP_STD.

    return std + (cxxflags.empty() ? "" : " " + vector_to_string(cxxflags));
}

std::string Settings::gen_additional_flags() const { return vector_to_string(additional_flags); }

std::string Settings::gen_command_line_code() const { return merge_args(code_start, code_count); }

std::string Settings::gen_command_line_args() const {
    string args = "";
    for (int i = 0; i < args_count; i++) {
        char *arg = args_start[i];

        bool has_space = false;
        char *p = arg;
        while (*p) {
            if (isspace(*p)) {
                has_space = true;
                break;
            }
        }

        if (has_space) {
            args += "\"" + string(arg) + "\" ";
        } else {
            args += string(arg) + " ";
        }
    }

    if (args.size() > 0) {
        args.pop_back(); // Remove the trailing space
    }

    return args;
}

bool Settings::has_code() const { return code_count > 0; }

void Settings::debug_print(std::ostream &os) const {
    os << "Settings:" << endl;
    os << "  compiler: " << compiler << endl;
    os << "  cxxflags: " << gen_cxxflags() << endl;
    os << "  additional_flags: " << gen_additional_flags() << endl;

    os << "  code_count: " << code_count << endl;
    os << "  args_count: " << args_count << endl;

    os << "  clean_cache: " << clean_cache << endl;
}

} // namespace rcc