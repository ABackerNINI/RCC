#include "settings.h"

#include <cstring>

#include "libs/CLI11.hpp"
#include "libs/debug.h"
#include "libs/rang.hpp"

using namespace std;

namespace rcc {

static int locate_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            return i;
        }
    }
    return argc;
}

// Merge all user codes together into a string.
string Settings::merge_codes(const std::vector<std::string> &codes) {
    string s;
    s.reserve(256);
    // If there is only one argument and it doesn't end with ';', wrap it in
    // 'cout << ... << endl;' directly. This is for convenience, e.g. rcc '2+3*5'.
    if (codes.size() == 1 && codes[0].length() > 0 && codes[0][codes[0].length() - 1] != ';') {
        s += "cout << " + codes[0] + " << endl;";
    } else {
        // Otherwise, merge all arguments together without any modification.
        for (auto &code : codes) {
            s.append(code);
        }
    }
    return s;
}

int Settings::parse_argv(int argc, char **argv) {
    int args_index = locate_args(argc, argv);

    if (args_index < argc) {
        args_start = &argv[args_index + 1];
        args_count = argc - args_index - 1;
        argc -= args_count + 1;
    }

    CLI::App app{"RCC - Run C/C++ codes in terminal"};
    app.allow_non_standard_option_names();
    app.allow_extras();
    app.add_flag("--clean-cache", clean_cache, "Clean cached source and binary files");
    app.add_option_function<string>(
           "--include",
           [&](const string &inc) {
               if (inc == "bits/stdc++.h") {
                   included_stdcpp = true;
               }
               additional_includes.push_back(inc);
           },
           "Include additional header")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse();
    app.add_flag_callback(
        "--include-all",
        [&]() {
            included_stdcpp = true;
            additional_includes.push_back("bits/stdc++.h");
        },
        "Include the bits/stdc++.h header, this will increase compile time");
    app.add_option_function<string>(
           "--compile-with",
           [&](const string &fname) { additional_sources.push_back(fname); },
           "Compile with additional source file")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse();
    app.add_option_function<string>(
           "--put-above-main",
           [&](const string &code) { above_main.push_back(code); },
           "Any code that should be put above the main function")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse();
    app.add_option_function<string>(
           "--function",
           [&](const string &code) { functions.push_back(code); },
           "Define a function")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        std::cout << (e.get_exit_code() == 0 ? rang::fg::blue : rang::fg::red);
        return app.exit(e);
    }

    vector<string> remaining = app.remaining(true);

    for (auto &s : remaining) {
        if (s.substr(0, 1) == "-") {
            if (s.substr(0, 4) == "-std") {
                std = s;
            } else if (s.substr(0, 2) == "-l" || s == "-math" || s == "-pthread") {
                additional_flags.push_back(s);
            } else {
                cxxflags.push_back(s);
            }
        } else {
            codes.push_back(s);
        }
    }

    return 0;
}

std::string Settings::get_cxxflags_as_string() const {
    //? Should use a C++ standard like "-std=c++11"?
    //* This will be necessary on some lower version compilers. But this will
    //* prevent us using a higher default standard.
    //* The current solution is test C++ standard before making rcc, then use
    //* the highest possible standard to compile the rcc and template header as
    //* well as use it here. See RCC_CPP_STD.

    return std + (cxxflags.empty() ? "" : " " + vector_to_string(cxxflags));
}

std::string Settings::get_cli_args_as_string() const {
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
            ++p;
        }

        if (has_space) {
            // TODO: maybe wrap the arg in single or double quotes depending on whether it
            // contains single or double quotes.
            args += "'" + string(arg) + "' ";
        } else {
            args += string(arg) + " ";
        }
    }

    if (args.size() > 0) {
        args.pop_back(); // remove the trailing space
    }

    return args;
}

void Settings::debug_print() const {
    cdbg << "Settings:" << endl;
    cdbgc << "compiler: " << compiler << endl;
    cdbgc << "std: " << std << endl;
    cdbgc << "cxxflags: " << get_cxxflags_as_string() << endl;
    cdbgc << "additional_flags: " << get_additional_flags_as_string() << endl;
    cdbgc << "additional_includes: " << vector_to_string(additional_includes) << endl;
    cdbgc << "above_main_count: " << above_main.size() << endl;
    cdbgc << "functions_count: " << functions.size() << endl;
    cdbgc << "code_count: " << codes.size() << endl;
    cdbgc << "additional_sources: " << vector_to_string(additional_sources) << endl;
    cdbgc << "args_count: " << args_count << endl;

    cdbgc << "clean_cache: " << clean_cache << endl;
}

std::string Settings::vector_to_string(const std::vector<std::string> &vec, const std::string &sep) {
    string result = "";
    for (const auto &item : vec) {
        result += item + sep;
    }
    if (result.size() > 0) {
        result.pop_back();
    }
    return result;
}

} // namespace rcc