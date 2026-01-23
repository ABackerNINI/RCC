#include "compiler_support.h"
#include "debug_fmt.h"
#include "paths.h"
#include "settings.h"
#include "utils.h"
#include <csignal>
#include <iostream>

using namespace rcc;

// TODO: Handle signals

// Signal handler for SIGINT (Control-C) to exit the program gracefully.
void signal_handler(int s) {
    (void)s;
    std::cout << "Control-C detected, exiting..." << std::endl;
    std::exit(1);
}

// Register signal handler for SIGINT to exit the program gracefully.
void register_signal_handler() {
    // Nice Control-C
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}

// Delete old cached files with probability of 1/256.
pid_t random_clean_cache() {
    // 1/256 chances
    //? Why not use the hash value of code as the seed?
    //* The srand() and rand() is fast enough. And the same code has the same hash value.
    if (rand() % 256 == 0) {
        // Flush stdout and stderr before fork() to avoid duplicate output.
        fflush(stdout);
        fflush(stderr);

        pid_t pid = fork();
        if (pid == 0) { // in child process
            const Paths &paths = Paths::get_instance();
            // Find and remove src/bin files whose access time is 31 days ago
            //! Caution: rm command
            std::string
                find_rm_cmd = format("find {} -type f \\( -name \"*.cpp\" -o -name \"*.bin\" \\) -atime +30 -delete",
                                     paths.get_sub_cache_dir().quote_if_needed());

            const auto tty_ts = TTY_TS(fg(color::dark_red) | emphasis::bold, stderr);
            gpdebug("{}: {}\n", styled("Removing old cache files", tty_ts), find_rm_cmd);

            if (system(find_rm_cmd.c_str()) != 0) {
                perror("system:find");
                exit(1);
            }

            // Leave this process to the init
            exit(0);
        } else if (pid < 0) {
            perror("fork");
        }
        return pid;
    }
    return -1;
}

// Clean up all cached sources and binaries.
int clean_cache() {
    //! Caution: rm command
    const std::string sub_cache_dir = Paths::get_instance().get_sub_cache_dir().quote_if_needed();
    std::string rm_cmd = "rm -f " + sub_cache_dir + "/*.cpp " + sub_cache_dir + "/*.bin";
    return system(rm_cmd.c_str());
}

// Check if the binary is cached and the content matches.
//* The file hash may collide, so we need to check the content as well.
bool check_if_cached(const Path &bin_path, const Path &cpp_path, const std::string &full_code) {
    if (bin_path.exists()) {
        const std::string code_old = cpp_path.read_file();
        if (code_old == full_code) {
            return true;
        }

        gpdebug(TTY_TS(red_bold, stderr), "WARNING: hash collided but content does not match!\n");
        gpdebug("{}:\n{}", styled("Old Code", TTY_TS(fg(color::red), stderr)), code_old);
        gpdebug("{}:\n{}", styled("New Code", TTY_TS(fg(color::red), stderr)), full_code);
    }
    return false;
}

// Generate the execution command with the binary path and command line arguments.
std::string gen_exec_cmd(const Settings &settings, const Path &bin_path) {
    const std::string command_line_args = settings.get_cli_args_as_string();
    const std::string exec_cmd = bin_path.quote_if_needed() +
                                 (command_line_args.empty() ? "" : " " + command_line_args);
    return exec_cmd;
}

// Compile the code.
bool compile_code(const Settings &settings,
                  const Path &bin_path,
                  const Path &cpp_path,
                  const compiler_support &cs,
                  bool silent = false) {
    std::vector<Path> sources = {cpp_path};
    for (auto &src : settings.get_additional_sources()) {
        sources.emplace_back(src);
    }

    const std::string compile_cmd = cs.get_compile_command(sources, bin_path) + (silent ? " >/dev/null 2>&1" : "");

    gpdebug("{}\n", compile_cmd);

    using namespace fmt;

    if (system(compile_cmd) != 0) {
        if (!silent) {
            const std::string exec_cmd = gen_exec_cmd(settings, bin_path);
            print(stderr, TTY_TS(red_bold, stderr), "\nCOMPILATION FAILED!\n");
            if (isatty(fileno(stderr))) {
                print(stderr, "OUTPUT CPP: \e]8;;file://{}\a{}\e]8;;\a\n", cpp_path.quote_if_needed(), "file");
            } else {
                print(stderr, "OUTPUT CPP: file://{}\n", cpp_path.quote_if_needed());
            }
            const auto tty_saddle_brown_bold = TTY_TS(fg(color::saddle_brown) | emphasis::bold, stderr);
            gpdebug("{}: {}\n", styled("COMPILE COMMAND", tty_saddle_brown_bold), compile_cmd);
            gpdebug("{}: {}\n", styled("EXECUTE COMMAND", tty_saddle_brown_bold), exec_cmd);
        }
        return false;
    }
    return true;
}

// Run the binary executable, return the return code of the executable.
int run_bin(const Settings &settings, const Path &cpp_path, const Path &bin_path) {
    const std::string exec_cmd = gen_exec_cmd(settings, bin_path);

    /*------------------------------------------------------------------------*/
    // * Run the Executable

    gpdebug("OUTPUT CPP: \e]8;;file://{}\a{}\e]8;;\a\n", cpp_path.quote_if_needed(), "file");
    gpdebug("EXECUTING : {}\n", styled(exec_cmd, TTY_TS(emphasis::underline, stderr)));

    const auto tty_yellow_bold = TTY_TS(fg(color::yellow) | emphasis::bold, stderr);
    gpdebug(tty_yellow_bold, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    int ret = system(exec_cmd);
    gpdebug(tty_yellow_bold, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

    const auto tty_red = TTY_TS(fg(color::red), stderr);

    if (ret == -1) { // System call failed. This is an error, e.g. fork() failed
        print(stderr, tty_red, "system(): {}\n", strerror(errno));
        return 1;
    };

    int exit_status = 1;
    if (WIFEXITED(ret)) { // The process exited normally
        exit_status = WEXITSTATUS(ret);
    } else if (WIFSIGNALED(ret)) { // The process was terminated by a signal
        print(stderr, tty_red, "Killed by signal {}\n", WTERMSIG(ret));
        return 1;
    } else { // The process was stopped by a signal or some other unexpected event
        print(stderr, tty_red, "Unexpected exit status {}\n", ret);
        return 1;
    }

    return exit_status;
}

// Suggest a similar permanent, return empty string if not match found.
std::string suggest_similar_permanent(const std::string &name) {
    if (name.size() <= 1) {
        return "";
    }

    const Paths &paths = Paths::get_instance();

    std::string suggestion;

    try {
        auto files = find_files(paths.get_sub_permanent_dir().get_path(), {".bin"});
        gpdebug("Found {} files\n", files.size());

        // Find the closest match
        size_t min_distance = INT32_MAX;
        for (auto &file : files) {
            size_t distance = edit_distance(name, file.stem());
            if ((distance <= name.size() / 2 && distance <= file.stem().string().size() / 2) &&
                distance < min_distance) {
                min_distance = distance;
                suggestion = file.stem();
            }
        }
    } catch (const fs::filesystem_error &e) {
        gpdebug("Filesystem error: {}\n", e.what());
        return "";
    } catch (const std::exception &e) {
        gpdebug("Error: {}\n", e.what());
        return "";
    }

    return suggestion;
}

// Run a permanent executable, return the return code of the executable or 1 if the executable does not exist.
int run_permanent(const Settings &settings, const std::string &name) {
    // rcc paths
    const Paths &paths = Paths::get_instance();

    Path cpp_path, bin_path, desc_path;
    paths.get_src_bin_full_path_permanent(name, cpp_path, bin_path, desc_path);

    if (!bin_path.exists()) {
        // If the binary does not exist but the source or description file exist, it's an invalid permanent.
        // So we don't need to suggest anything.
        if (cpp_path.exists() || desc_path.exists()) {
            print(stderr,
                  "Error: the binary of permanent '{}' does not exist. It's likely due to a compilation failure "
                  "earlier.\n",
                  styled(name, TTY_TS(red_bold, stderr)));
            return 1;
        }

        // Try to suggest similar names
        const std::string suggestion = suggest_similar_permanent(name);
        print(stderr, "Error: permanent '{}' does not exist", styled(name, TTY_TS(red_bold, stderr)));
        if (!suggestion.empty()) {
            print(stderr, ", did you mean '{}'?\n", styled(suggestion, TTY_TS(green_bold, stderr)));
        } else {
            print(stderr, ".\n");
        }

        return 1;
    }

    // Run the executable
    return run_bin(settings, cpp_path, bin_path);
}

// List all permanent executables, return 1 on error.
int list_permanent(const Settings &settings) {
    // rcc paths
    const Paths &paths = Paths::get_instance();

    try {
        auto files = find_files(paths.get_sub_permanent_dir().get_path(), {".desc"});
        gpdebug("Found {} files\n", files.size());

        for (const auto &file : files) {
            std::string desc = Path(file).read_file();

            if (settings.get_flag_fetch_autocompletion_zsh()) {
                // No color, no space between name and description
                print("{}:{}\n", file.stem().string(), desc);
            } else {
                Path cpp_path, bin_path, desc_path;
                paths.get_src_bin_full_path_permanent(file.stem(), cpp_path, bin_path, desc_path);
                // Show in red if the binary doesn't exist (compilation failed or has been deleted)
                auto color = bin_path.exists() ? terminal_color::green : terminal_color::red;
                print("{}: {}\n", styled(file.stem().string(), TTY_TS(fg(color))), desc);
            }
        }
    } catch (const fs::filesystem_error &e) {
        print(stderr, "Filesystem error: {}\n", e.what());
        return 1;
    } catch (const std::exception &e) {
        print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}

// Remove file and handle exceptions. Return true if successful, false otherwise.
bool remove_file(Path &p) noexcept {
    try {
        // *Note: remove() does not throw if the file does not exist. It returns false in that case.
        // *Note: remove() may throw `std::bad_alloc` or `fs::filesystem_error`
        return p.remove();
    } catch (const std::exception &e) {
        gperror("{}", e.what());
        return false;
    }
}

// Remove permanent files, return 0 if all files were removed successfully, 1 otherwise.
int remove_permanents(const Settings &settings) {
    // rcc paths
    const Paths &paths = Paths::get_instance();

    int ret = 0;
    size_t num_removed = 0;
    for (const auto &permanent : settings.get_remove_permanent()) {
        Path cpp_path, bin_path, desc_path;
        paths.get_src_bin_full_path_permanent(permanent, cpp_path, bin_path, desc_path);

        bool success = false;

        success |= remove_file(cpp_path);
        success |= remove_file(bin_path);
        success |= remove_file(desc_path);

        if (success) {
            ++num_removed;
            gpdebug("Removed '{}'\n", permanent);
        } else {
            ret = 1;
            gperror("Permanent '{}' does not exist!\n", styled(permanent, TTY_TS(fg(terminal_color::red), stderr)));
        }
    }

    print("Removed {} permanent(s), {} failed.\n", num_removed, settings.get_remove_permanent().size() - num_removed);

    return ret;
}

struct TryCodeResult {
    enum TryStatus { SUCCESS, COMPILE_FAILED, ERROR };

    TryStatus status;
    int exit_status;
};

// Silent mode: no output of compiler errors, and no output after the compilation failed.
TryCodeResult try_code(const Settings &settings, const std::string &code, bool silent = false) {
    // rcc paths
    const Paths &paths = Paths::get_instance();

    // the compiler
    const std::string compiler = settings.get_compiler();

    const std::string cxxflags = settings.get_std_cxxflags_as_string();
    const std::string additional_flags = settings.get_additional_flags_as_string();
    const std::string additional_includes = settings.get_additional_includes_as_string();
    const std::string above_main = settings.get_above_main_as_string();
    const std::string functions = settings.get_functions_as_string();
    const std::string additional_sources = settings.get_additional_sources_as_string();

    // the output cpp code and executable file's full paths
    Path cpp_path, bin_path, desc_path;
    bool desc_written = false;

    if (settings.get_permanent().empty()) {
        // The string to hash, which determines the output file name.
        // It is used to determine if we need to recompile the code or not.
        const std::string to_hash = code + "a" + compiler + "b" + cxxflags + "a" + additional_flags + "c" +
                                    additional_includes + "k" + above_main + "e" + functions + "r" + additional_sources;

        paths.get_src_bin_full_path(to_hash, cpp_path, bin_path);
    } else {
        paths.get_src_bin_full_path_permanent(settings.get_permanent(), cpp_path, bin_path, desc_path);

        // Write the description of the permanent
        // If the description file doesn't exist, we can safely write it before the compilation
        if (!desc_path.exists() && !settings.get_permanent_desc().empty()) {
            desc_path.write_file(settings.get_permanent_desc());
            desc_written = true;
        }
    }

    // the compiler support
    auto cs = create_compiler_support(compiler, settings);

    // The hash of this string will be written into the cpp file, so that if one of the fields change, we can detect it
    // and recompile the code. This is an insurance in case the first hash collides.
    //* So in theory, if this program somehow runs the wrong binary, then the two different inputs must have the same
    //* two hashes, and the same code, includes, above main, and functions, since these fields will go into the cpp file
    //* as well, and as what they were given.
    const std::string id = compiler + "n" + cxxflags + "i" + additional_flags + "n" + additional_includes + "i" +
                           additional_sources;

    // full c++ code generated by the template and the command line arguments
    const std::string full_code = cs->gen_code(paths.get_template_file_path(), settings.get_additional_includes(),
                                               settings.get_above_main(), settings.get_functions(), code,
                                               u64_to_string_base64x(fnv1a_64_hash_string(id)));

    /*------------------------------------------------------------------------*/
    // * Compile If Needed

    if (settings.get_permanent().empty() && check_if_cached(bin_path, cpp_path, full_code)) {
        // Cached, skip the compiling process, run the executable directly
        gpdebug(TTY_TS(fg(color::green), stderr), "Running cached binary\n");
    } else {
        if (!settings.get_permanent().empty()) {
            // TODO: confirm overwrite, add option -f, --force
        }

        // Write c++ code to the cpp file
        cpp_path.write_file(full_code);

        if (!compile_code(settings, bin_path, cpp_path, *cs, silent)) {
            return {TryCodeResult::COMPILE_FAILED, 1}; // Compile failed
        }
    }

    // If --permanent is set, just compile the code, don't run it
    if (!settings.get_permanent().empty()) {
        // If compilation succeeded, we need to update the description
        // * This means if the compilation failed, we do NOT update the description
        if (!desc_written) {
            desc_path.write_file(settings.get_permanent_desc().empty() ? "No description provided"
                                                                       : settings.get_permanent_desc());
        }

        return {TryCodeResult::SUCCESS, 0};
    }

    /*------------------------------------------------------------------------*/
    // * Run the Executable

    int ret = run_bin(settings, cpp_path, bin_path);

    return {TryCodeResult::SUCCESS, ret};
}

struct AutoWrapResult {
    bool tried;
    TryCodeResult try_result;

    // For C++11 compatibility
    AutoWrapResult(bool tried = false, TryCodeResult try_result = TryCodeResult())
        : tried(tried), try_result(try_result) {}
};

AutoWrapResult auto_wrap(const Settings &settings) {
    auto time_begin = now();

    auto &codes = settings.get_codes();

    if (codes.empty()) {
        return {false, {}}; // No code to wrap
    }

    // If the last code snippet doesn't end with ';' or '}', then, wrap it in
    // 'cout << ... << endl;' and try to compile and run it.
    // This is for convenience, e.g. rcc '2+3*5'.

    auto &last_code = codes.back();
    if (last_code.length() > 0 && last_code.back() != ';' && last_code.back() != '}') {
        std::string code;

        for (size_t i = 0; i < codes.size() - 1; i++) {
            code.append(codes[i]);
        }
        code.append("cout << (" + last_code + ") << endl;");

        const auto tty_dodger_blue_bold = TTY_TS(fg(color::dodger_blue) | emphasis::bold, stderr);

        gpdebug(tty_dodger_blue_bold, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        gpdebug("Trying to wrap code with 'cout << ... << endl;'\n");
        auto try_result = try_code(settings, code, true); // silent mode
        if (try_result.status != TryCodeResult::SUCCESS) {
            gpdebug(TTY_TS(fg(color::brown) | emphasis::bold, stderr), "AUTO-WRAP FAILED\n");
        }
        gpdebug(tty_dodger_blue_bold, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
        gpinfo("Auto-wrapping took {:.2f} ms\n", duration_ms(time_begin, now()));

        return {true, try_result};
    }

    return {false, {}};
}

// The main function of rcc.
// Convenient for testing.
int rcc_main(int argc, char **argv) {
    // TODO: add options --g++ --clang++
    // TODO: --error-exitcode=<number> exit code to return if errors found [0=disable]
    // TODO: add -q, --quiet, or --silent flag to suppress output
    // TODO: add the fmt, ghc libraries
    // TODO: no color for non-tty
    // TODO: add json support
    // TODO: add csv support
    // TODO: add version info
    // TODO: add option, --debug, show debug messages
    // TODO: add option -c, --compile-only, compile only, return binary's name, run later
    // TODO: add option --dry-run, show what would be done without actually doing it
    // TODO: rcc_print, rcc_print_pretty: print c++ code only
    // TODO: add option --pretty
    // TODO: add option cat, cat last generated code
    // TODO: add option --print only
    // TODO: add support for Windows
    // TODO: make install_local: install it at the current directory
    // TODO: auto detect compiler
    // TODO: check template files during make install
    // TODO: store in .local, reproduce install if .cache/rcc files been removed
    // TODO: add a readme in .cache/rcc
    // TODO: add version and help messages
    // TODO: boost with multi-thread
    //? TODO: add option --stdin, read input from stdin instead of arguments

    // Handle signals gracefully
    register_signal_handler();

    // Parse arguments and set up settings
    Settings settings;
    int result;
    if ((result = settings.parse_argv(argc, argv)) != 0) {
        return result;
    }

    // Seed the random number generator
    //? Why time() + getpid()?
    //* If we only use time() as the seed, rcc may run in one second multiple
    //* times, and get the same seed.
    srand((unsigned int)time(NULL) + (unsigned int)getpid());

    // Clean old cached files
    if (settings.get_clean_cache_flag()) { // clean cache manually
        clean_cache();
    } else { // clean cache automatically, but not too often
        random_clean_cache();
    }

    // If --list-permanent is set, list all permanent programs
    if (settings.get_flag_list_permanent()) {
        return list_permanent(settings);
    }

    // If --remove-permanent is set, remove the specified permanents
    if (!settings.get_remove_permanent().empty()) {
        return remove_permanents(settings);
    }

    // If --run-permanent is set, just run the program
    if (!settings.get_run_permanent().empty()) {
        return run_permanent(settings, settings.get_run_permanent());
    }

    // No code to compile, just return
    if (!settings.has_code()) {
        if (!settings.get_permanent().empty()) {
            gperror("No code to compile as permanent program.\n");
            return 1;
        }
        return 0;
    }

    // Try to auto-wrap the code, if successful, return the result directly
    auto auto_warp_result = auto_wrap(settings);
    if (auto_warp_result.tried && auto_warp_result.try_result.status == TryCodeResult::SUCCESS) {
        return auto_warp_result.try_result.exit_status;
    }

    // command line c++ code
    const std::string code = settings.get_codes_as_string();

    // Try to compile and run the unwrapped code
    auto try_result = try_code(settings, code);

    return try_result.exit_status;
}

int main(int argc, char **argv) {
    return rcc_main(argc, argv);
}
