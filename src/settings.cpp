#include "pch.h"

#include "debug_fmt.h"
#include "settings.h"

namespace rcc {

int Settings::locate_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            return i;
        }
    }
    return argc;
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
        ->trigger_on_parse()
        ->check(CLI::ExistingFile);
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
    app.add_option_function<string>(
           "--code",
           [&](const string &code) { codes.push_back(code); },
           "Add code explicitly")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        std::cout << (e.get_exit_code() == 0 ? rang::fg::blue : rang::fg::red);
        int ret = app.exit(e);
        std::cout << rang::fg::reset;
        return ret;
    }

    vector<string> remaining = app.remaining(true);

    for (auto &s : remaining) {
        if (s.substr(0, 1) == "-" && s.size() >= 2 && std::isalpha(s[1])) { // Option or flag
            if (s.substr(0, 5) == "-std=") {
                std = s;
            } else if (s.substr(0, 2) == "-l") {
                additional_flags.push_back(s);
            } else {
                if (codes.empty()) { // If no code has been added yet, add it to cxxflags
                    cxxflags.push_back(s);
                } else { // Otherwise add it to additional flags
                    additional_flags.push_back(s);
                }
            }
        } else { // Code
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
    const string cxxflags = get_cxxflags_as_string();
    const string additional_flags = get_additional_flags_as_string();

    gprint("Settings:\n");
    gprintc("compiler: {}\n", compiler);
    gprintc("std: {}\n", std);
    gprintc("cxxflags: {}\n", cxxflags.empty() ? "<NONE>" : cxxflags);
    gprintc("additional_flags: {}\n", additional_flags.empty() ? "<NONE>" : additional_flags);
    gprintc("additional_includes: {}\n", vector_to_string(additional_includes, ", ", "<NONE>"));
    gprintc("above_main_count: {}\n", above_main.size());
    gprintc("functions_count: {}\n", functions.size());
    gprintc("code_count: {}\n", codes.size());
    gprintc("additional_sources: {}\n", vector_to_string(additional_sources, ", ", "<NONE>"));
    gprintc("args_count: {}\n", args_count);
    gprintc("clean_cache: {}\n", clean_cache);
}

} // namespace rcc
