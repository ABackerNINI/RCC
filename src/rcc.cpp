#include "rcc.h"
#include "code.h"
#include "compiler_support.h"
#include "debug_fmt.h"
#include "paths.h"
#include "settings.h"
#include "utils.h"
#include <csignal>
#include <iostream>

namespace rcc {

static const std::string DEFAULT_PERMANENT_DESC = "No description provided";

pid_t RCC::random_clean_cache() {
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
            std::string find_rm_cmd =
                format("find {} -type f \\( -name \"*.cpp\" -o -name \"*.bin\" \\) -atime +30 -delete",
                       paths.get_sub_cache_dir().quote_if_needed());

            const auto ts = fg(color::dark_red) | emphasis::bold;
            gpdebug("{}: {}\n", styled("Removing old cache files", ts), find_rm_cmd);

            if (system_s(find_rm_cmd) != 0) {
                gperror("system(): {}\n", strerror(errno));
                exit(1);
            }

            // Leave this process to the init
            exit(0);
        } else if (pid < 0) {
            gpwarning("fork(): {}\n", strerror(errno));
            // Note: fork failed, continue anyway
        }
        return pid;
    }
    return -1;
}

int RCC::clean_cache() {
    //! Caution: rm command
    const std::string sub_cache_dir = Paths::get_instance().get_sub_cache_dir().quote_if_needed();
    std::string rm_cmd = "rm -f " + sub_cache_dir + "/*.cpp " + sub_cache_dir + "/*.bin";
    return system_s(rm_cmd);
}

std::string RCC::gen_exec_cmd(const Settings &settings, const Path &bin_path) {
    const std::string command_line_args = settings.get_cli_args_as_string();
    const std::string exec_cmd = bin_path.quote_if_needed() +
                                 (command_line_args.empty() ? "" : " " + command_line_args);
    return exec_cmd;
}

int RCC::run_bin(const Settings &settings, const Path &cpp_path, const Path &bin_path) {
    const std::string exec_cmd = RCC::gen_exec_cmd(settings, bin_path);

    /*------------------------------------------------------------------------*/
    // * Run the Executable

    if (isatty(STDERR_FILENO)) {
        // This creates a hyperlink to the file in the terminal, only tested on zsh
        gpdebug("SRC FILE: \e]8;;file://{}\a{}\e]8;;\a\n", cpp_path.quote_if_needed(), "file");
    } else {
        // This creates a hyperlink to the file
        gpdebug("SRC FILE: file://{}\n", cpp_path.quote_if_needed());
    }
    gpdebug("EXECUTING: {}\n", styled(exec_cmd, emphasis::underline));

    const auto time_begin = now();

    const auto yellow_bold = fg(color::yellow) | emphasis::bold;
    gpdebug(yellow_bold, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    int ret = system_s(exec_cmd);
    gpdebug(yellow_bold, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    gpdebug("RUNNING TIME: {:.2f} ms\n", duration_ms(time_begin));

    if (ret == -1) { // System call failed. This is an error, e.g. fork() failed
        gperror("system(): {}\n", strerror(errno));
        return 1;
    }

    int exit_status = 1;
    if (WIFEXITED(ret)) { // The process exited normally
        exit_status = WEXITSTATUS(ret);
    } else if (WIFSIGNALED(ret)) { // The process was terminated by a signal
        gperror_ex(red_bold, "Killed by signal {}\n", WTERMSIG(ret));
        return 1;
    } else { // The process was stopped by a signal or some other unexpected event
        gperror_ex(red_bold, "Unexpected exit status {}\n", ret);
        return 1;
    }

    return exit_status;
}

bool RCC::compile_file(const Settings &settings,
                       const Path &cpp_path,
                       const Path &bin_path,
                       compiler_support &cs,
                       bool silent) {
    std::vector<Path> sources = {cpp_path};
    for (auto &src : settings.get_additional_sources()) {
        sources.emplace_back(src);
    }

    const std::string compile_cmd = cs.get_compile_command(sources, bin_path) + (silent ? " >/dev/null 2>&1" : "");

    gpdebug("{}\n", compile_cmd);

    if (system_s(compile_cmd) != 0) {
        if (!silent) {
            const std::string exec_cmd = RCC::gen_exec_cmd(settings, bin_path);
            if (isatty(fileno(stderr))) {
                // This creates a hyperlink to the file in the terminal, only tested on zsh
                gpwarning_ex("SRC FILE: \e]8;;file://{}\a{}\e]8;;\a\n", cpp_path.quote_if_needed(), "file");
            } else {
                // This creates a hyperlink to the file
                gpwarning_ex("SRC FILE: file://{}\n", cpp_path.quote_if_needed());
            }
            gperror_ex(red_bold, "\nCOMPILATION FAILED!\n");
            const auto ts = fg(color::saddle_brown) | emphasis::bold;
            gpdebug("{}: {}\n", styled("COMPILE COMMAND", ts), compile_cmd);
            gpdebug("{}: {}\n", styled("EXECUTE COMMAND", ts), exec_cmd);
        }
        return false;
    }
    return true;
}

std::string RCC::gen_first_hash_filename(const Settings &settings, const std::string &code) {
    const std::string &compiler = settings.get_compiler();

    const std::string cxxflags = settings.get_std_cxxflags_as_string();
    const std::string additional_flags = settings.get_additional_flags_as_string();
    const std::string additional_includes = settings.get_additional_includes_as_string();
    const std::string above_main = settings.get_above_main_as_string();
    const std::string functions = settings.get_functions_as_string();
    const std::string additional_sources = settings.get_additional_sources_as_string();

    // The string to hash, which determines the output file name.
    // It is used to determine if we need to recompile the code or not.
    const std::string to_hash = code + "a" + compiler + "b" + cxxflags + "a" + additional_flags + "c" +
                                additional_includes + "k" + above_main + "e" + functions + "r" + additional_sources;

    return u64_to_string_base64x(fnv1a_64_hash_string(to_hash));
}

std::string RCC::gen_second_hash_identifier(const Settings &settings) {
    const std::string &compiler = settings.get_compiler();

    const std::string cxxflags = settings.get_std_cxxflags_as_string();
    const std::string additional_flags = settings.get_additional_flags_as_string();
    const std::string additional_includes = settings.get_additional_includes_as_string();
    const std::string additional_sources = settings.get_additional_sources_as_string();

    // The hash of this string will be written into the cpp file, so that if one of the fields change, we can detect it
    // and recompile the code. This is an insurance in case the first hash collides.
    //* So in theory, if this program somehow runs the wrong binary, it means the two different inputs must have the
    //* same two hashes, and the same code, includes, above main, and functions, since these fields will go into the cpp
    //* file as well, and as what they were given.
    const std::string to_hash = compiler + "n" + cxxflags + "i" + additional_flags + "n" + additional_includes + "i" +
                                additional_sources;

    return u64_to_string_base64x(fnv1a_64_hash_string(to_hash));
}

std::string RCC::suggest_similar_permanent(const std::string &name) {
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
        gpwarning("{}\n", e.what());
        return "";
    } catch (const std::exception &e) {
        gpwarning("{}\n", e.what());
        return "";
    }

    return suggestion;
}

int RCC::run_permanent(const Settings &settings, const std::string &name) {
    // rcc paths
    const Paths &paths = Paths::get_instance();

    Path cpp_path, bin_path, desc_path;
    paths.get_src_bin_full_path_permanent(name, cpp_path, bin_path, desc_path);

    if (!bin_path.exists()) {
        // If the binary does not exist but the source or description file exist, it's an invalid permanent.
        // So we don't need to suggest anything.
        if (cpp_path.exists() || desc_path.exists()) {
            gperror_ex("Error: the binary of permanent '{}' does not exist. "
                       "It's likely due to a compilation failure earlier.\n",
                       styled(name, red_bold));
            return 1;
        }

        // Try to suggest similar names
        const std::string suggestion = suggest_similar_permanent(name);
        gperror_ex("Error: permanent '{}' does not exist", styled(name, red_bold));
        if (!suggestion.empty()) {
            gperror_ex(", did you mean '{}'?\n", styled(suggestion, green_bold));
        } else {
            gperror_ex(".\n");
        }

        return 1;
    }

    // Run the executable
    return run_bin(settings, cpp_path, bin_path);
}

int RCC::list_permanents(const Settings &settings) {
    // rcc paths
    const Paths &paths = Paths::get_instance();

    try {
        auto files = find_files(paths.get_sub_permanent_dir().get_path(), {".cpp"});
        gpdebug("Found {} files\n", files.size());

        for (const auto &file : files) {
            Path cpp_path, bin_path, desc_path;
            paths.get_src_bin_full_path_permanent(file.stem(), cpp_path, bin_path, desc_path);

            std::string desc = desc_path.exists() ? desc_path.read_file() : DEFAULT_PERMANENT_DESC;

            if (settings.get_flag_fetch_autocompletion_zsh()) {
                // No color, no space between name and description
                print("{}:{}\n", file.stem().string(), desc);
            } else {
                // Show in red if the binary doesn't exist (compilation failed or has been deleted)
                auto color = bin_path.exists() ? terminal_color::green : terminal_color::red;
                print("{}: {}\n", styled(file.stem().string(), fg(color)), desc);
            }
        }
    } catch (const fs::filesystem_error &e) {
        gperror("Filesystem error: {}\n", e.what());
        return 1;
    } catch (const std::exception &e) {
        gperror("{}\n", e.what());
        return 1;
    }

    return 0;
}

bool RCC::remove_file(Path &p) noexcept {
    try {
        // *Note: remove() does not throw if the file does not exist. It returns false in that case.
        // *Note: remove() may throw `std::bad_alloc` or `fs::filesystem_error`
        return p.remove();
    } catch (const std::exception &e) {
        gperror("{}", e.what());
        return false;
    }
}

int RCC::remove_permanents(const Settings &settings) {
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
            gperror("Permanent '{}' does not exist!\n", styled(permanent, fg(terminal_color::red)));
        }
    }

    print("Removed {} permanent(s), {} failed.\n", num_removed, settings.get_remove_permanent().size() - num_removed);

    return ret;
}

RCC::AutoWrapResult RCC::gen_auto_wrap_code(const Settings &settings) {
    auto &codes = settings.get_codes();

    if (codes.empty()) {
        return {false, {}}; // No code to wrap
    }

    auto &last_code = codes.back();
    if (last_code.length() > 0 && last_code.back() != ';' && last_code.back() != '}') {
        std::string code;

        for (size_t i = 0; i < codes.size() - 1; i++) {
            code.append(codes[i]);
        }
        code.append("cout << (" + last_code + ") << endl;");

        return {true, code};
    }

    return {false, {}}; // No need to wrap
}

RCC::TryCodeResult RCC::try_code_permanent(const Settings &settings) {
    const Paths &paths = Paths::get_instance();

    const std::string &code = settings.get_codes_as_string();

    // the output cpp code and executable file's full paths
    Path cpp_path, bin_path, desc_path;
    bool desc_written = false;

    paths.get_src_bin_full_path_permanent(settings.get_permanent(), cpp_path, bin_path, desc_path);

    // Write the description of the permanent
    // If the description file doesn't exist, we can safely write it before the compilation
    if (!desc_path.exists() && !settings.get_permanent_desc().empty()) {
        desc_path.write_file(settings.get_permanent_desc());
        desc_written = true;
    }

    // the compiler support
    auto cs = create_compiler_support(settings.get_compiler(), settings);

    // the identifier
    const std::string identifier = gen_second_hash_identifier(settings);

    // original code
    RCCodePermanent code_original(settings, paths, identifier, *cs, code, "original");
    code_original.init_cpp_bin_paths();

    /*------------------------------------------------------------------------*/
    // * Compile Anyway

    bool compile_success = false;

    // TODO: confirm overwrite, add option -f, --force

    // Try to compile the auto-wrapped code first
    const auto auto_warp = gen_auto_wrap_code(settings);
    if (auto_warp.tried) {
        RCCodePermanent code_auto_wrap(settings, paths, identifier, *cs, auto_warp.code, "auto-wrapped");
        code_auto_wrap.init_cpp_bin_paths();

        if (code_auto_wrap.compile(true)) {
            compile_success = true;
        }
    }

    // If the auto-wrapped code failed to compile, try the original code
    if (!compile_success && code_original.compile(false)) {
        compile_success = true;
    }

    if (!compile_success) {
        return {TryCodeResult::COMPILE_FAILED, 1}; // Both compilation failed
    }

    // If compilation succeeded, we need to update the description
    // * This means if the compilation failed, we do NOT update the description
    if (!desc_written) {
        desc_path.write_file(settings.get_permanent_desc().empty() ? DEFAULT_PERMANENT_DESC
                                                                   : settings.get_permanent_desc());
    }

    // Just compile the code, don't run it
    return {TryCodeResult::SUCCESS, 0};
}

RCC::TryCodeResult RCC::try_code_normal(const Settings &settings) {
    const Paths &paths = Paths::get_instance();

    // the original code
    const std::string &code = settings.get_codes_as_string();

    // the compiler support
    auto cs = create_compiler_support(settings.get_compiler(), settings);

    // the identifier
    const std::string identifier = gen_second_hash_identifier(settings);

    // the original code
    RCCode code_original(settings, paths, identifier, *cs, code, "original");
    code_original.init_cpp_bin_paths();

    // Check if the original code is cached. If so, just run it
    if (code_original.is_cached()) {
        gpdebug(green_bold, "Running cached binary ({})\n", "original");
        return {TryCodeResult::SUCCESS, code_original.run_bin()};
    }

    // Try to auto-wrap the code and try to compile it
    const auto auto_warp = gen_auto_wrap_code(settings);
    if (auto_warp.tried) {
        RCCode code_auto_wrap(settings, paths, identifier, *cs, auto_warp.code, "auto-wrapped");
        code_auto_wrap.init_cpp_bin_paths();

        // If the auto-wrapped code is cached, just run it
        if (code_auto_wrap.is_cached()) {
            gpdebug(green_bold, "Running cached binary ({})\n", "auto-wrapped");
            return {TryCodeResult::SUCCESS, code_auto_wrap.run_bin()};
        }

        // If the auto-wrapped code compiles successfully, run it
        if (code_auto_wrap.compile(true)) {
            return {TryCodeResult::SUCCESS, code_auto_wrap.run_bin()};
        }
    }

    // Compile and run the original code
    if (code_original.compile(false)) {
        return {TryCodeResult::SUCCESS, code_original.run_bin()};
    }

    // Both the auto-wrapped code and the original code failed to compile
    return {TryCodeResult::COMPILE_FAILED, 1};
}

RCC::TryCodeResult RCC::try_code(const Settings &settings) {
    return settings.get_permanent().empty() ? try_code_normal(settings) : try_code_permanent(settings);
}

// TODO: save CLI arguments of a permanent code
// TODO: --error-exitcode=<number> exit code to return if errors found [0=disable]
// TODO: add -q, --quiet, or --silent flag to suppress output
// TODO: add the fmt, ghc libraries
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
// TODO: boost with multi-thread
//? TODO: add option --stdin, read input from stdin instead of arguments

// The main function of rcc.
// Convenient for testing.
int RCC::rcc_main(const Settings &settings) {
    // Clean old cached files
    if (settings.get_clean_cache_flag()) { // clean cache manually
        clean_cache();
    } else { // clean cache automatically, but not too often
        random_clean_cache();
    }

    // If --list-permanent is set, list all permanent programs
    if (settings.get_flag_list_permanent()) {
        return list_permanents(settings);
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

    // Try to compile and run the code
    return try_code(settings).exit_status;
}

} // namespace rcc

//*==========================================================================*/

// TODO: Handle signals

// Signal handler for SIGINT (Control-C) to exit the program gracefully.
static void signal_handler(int s) {
    (void)s;
    std::cout << "Control-C detected, exiting..." << std::endl;
    std::exit(1);
}

// Register signal handler for SIGINT to exit the program gracefully.
static void register_signal_handler() {
    // Nice Control-C
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}

int main(int argc, char **argv) {
    using namespace rcc;

    const auto time_begin = now();

    // Handle signals gracefully
    register_signal_handler();

    // Seed the random number generator
    //? Why time() + getpid()?
    //* If we only use time() as the seed, rcc may run in one second multiple
    //* times, and get the same seed.
    srand((unsigned int)time(NULL) + (unsigned int)getpid());

    // static std::vector<DurationAndTS> dts = {{1.0, fg(terminal_color::green) | emphasis::bold},
    //                                          {1.0, fg(terminal_color::cyan) | emphasis::bold},
    //                                          {1.0, fg(terminal_color::yellow) | emphasis::bold},
    //                                          {1.0, fg(terminal_color::red) | emphasis::bold}};

    // Parse arguments and set up settings
    Settings settings;
    int result;
    if ((result = settings.parse_argv(argc, argv)) != 0) {
        const double duration_in_ms = duration_ms(time_begin);
        gpdebug("{}: {:.2f} ms\n", styled("OVERALL TIME", fg(terminal_color::yellow) | emphasis::bold),
                colored_duration(20, 100, duration_in_ms));
        return result;
    }

    int exit_status = RCC().rcc_main(settings);

    const double duration_in_ms = duration_ms(time_begin);
    gpdebug("{}: {:.2f} ms\n", styled("OVERALL TIME", fg(terminal_color::yellow) | emphasis::bold),
            colored_duration(80, 600, duration_in_ms));

    return exit_status;
}
