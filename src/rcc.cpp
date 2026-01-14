#include "pch.h"

// TODO: rearrange headers

#include "compiler_support.h"
#include "debug_fmt.h"
#include "paths.h"
#include "settings.h"
#include "utils.h"

using namespace rcc;

// Ignore the result of a function call so that compiler doesn't warn about
// unused return result.
template <typename T> void IGNORE_RESULT(T &&) {}

// Wrapper function of system(const char *);
int system(const string &cmd) {
    // print(cmd);
    return system(cmd.c_str());
}

// Wrapper for system() to ignore return value.
void ignore_system(const char *cmd) {
    // [[maybe_unused]] auto result = system(cmd); // ignore result
    IGNORE_RESULT(system(cmd));
}

// Wrapper for system() to ignore return value.
void ignore_system(const string &cmd) {
    // [[maybe_unused]] auto result = system(cmd); // ignore result
    IGNORE_RESULT(system(cmd));
}

// Signal handler for SIGINT (Control-C) to exit the program gracefully.
void signal_handler(int s) {
    (void)s;
    std::cout << std::endl << rang::style::reset << rang::fg::red << rang::style::bold;
    std::cout << "Control-C detected, exiting..." << rang::style::reset << std::endl;
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
            Paths &paths = Paths::get_instance();
            // find and remove src/bin files whose access time is 31 days ago
            // string find_rm_cmd;
            // find_rm_cmd += string("find ") + paths.get_sub_cache_dir().get_path();
            // find_rm_cmd += string(" -type f");
            // find_rm_cmd += string(" \\( -name \"*.cpp\" -o -name \"*.bin\" \\)");
            // find_rm_cmd += string(" -atime +30 | xargs rm -f");

            //! Caution: rm command
            string find_rm_cmd =
                fmt::format("find {} -type f \\( -name \"*.cpp\" -o -name \"*.bin\" \\) -atime +30 -delete",
                            paths.get_sub_cache_dir().quote_if_needed());

            gprint("{}: {}\n",
                   fmt::styled("Removing old cache files", fg(fmt::color::dark_red) | fmt::emphasis::bold),
                   find_rm_cmd);

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
    const string sub_cache_dir = Paths::get_instance().get_sub_cache_dir().quote_if_needed();
    string rm_cmd = "rm -f " + sub_cache_dir + "/*.cpp " + sub_cache_dir + "/*.bin";
    return system(rm_cmd.c_str());
}

// Check if the binary is cached and the content matches.
//* The file hash may collide, so we need to check the content as well.
bool check_if_cached(const Path &bin_path, const Path &cpp_path, const string &full_code) {
    if (bin_path.exists()) {
        const string code_old = cpp_path.read_file();
        if (code_old == full_code) {
            return true;
        }
        gprint(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "WARNING: hash collided but content does not match!\n");
        gprint("{}:\n{}", fmt::styled("Old Code", fmt::fg(fmt::color::red)), code_old);
        gprint("{}:\n{}", fmt::styled("New Code", fmt::fg(fmt::color::red)), full_code);
    }
    return false;
}

// Generate the execution command with the binary path and command line arguments.
std::string gen_exec_cmd(const Settings &settings, const Path &bin_path) {
    const string command_line_args = settings.get_cli_args_as_string();
    const string exec_cmd = bin_path.quote_if_needed() + (command_line_args.empty() ? "" : " " + command_line_args);
    return exec_cmd;
}

// Compile the code.
bool compile_code(const Settings &settings,
                  const Path &bin_path,
                  const Path &cpp_path,
                  const string &cxxflags,
                  const string &additional_flags,
                  std::shared_ptr<compiler_support> cs,
                  bool silent = false) {
    vector<Path> sources = {cpp_path};
    for (auto &src : settings.get_additional_sources()) {
        sources.emplace_back(src);
    }

    const string compile_cmd = cs->get_compile_command(sources, bin_path, cxxflags, additional_flags) +
                               (silent ? " >/dev/null 2>&1" : "");

    gprint("{}\n", compile_cmd);

    using namespace fmt;

    if (system(compile_cmd) != 0) {
        if (!silent) {
            const std::string exec_cmd = gen_exec_cmd(settings, bin_path);
            // print(emphasis::underline, "\n{}\n", cpp_path.quote_if_needed());
            print("OUTPUT CPP: \e]8;;file://{}\a{}\e]8;;\a\n", cpp_path.quote_if_needed(), "file");
            print(fg(color::red) | emphasis::bold, "\nCOMPILATION FAILED!\n");
            print("{}: {}\n", styled("COMPILE COMMAND", fg(color::saddle_brown) | emphasis::bold), compile_cmd);
            print("{}: {}\n", styled("EXECUTE COMMAND", fg(color::saddle_brown) | emphasis::bold), exec_cmd);
        }
        return false;
    }
    return true;
}

// Run the binary executable, return the return code of the executable.
int run_bin(const Settings &settings, const Path &cpp_path, const Path &bin_path) {
    const string exec_cmd = gen_exec_cmd(settings, bin_path);

    /*------------------------------------------------------------------------*/
    // * Run the Executable

    gprint("OUTPUT CPP: \e]8;;file://{}\a{}\e]8;;\a\n", cpp_path.quote_if_needed(), "file");
    gprint("EXECUTING : {}\n", fmt::styled(exec_cmd, fmt::emphasis::underline));

    gprint(fg(fmt::color::yellow) | fmt::emphasis::bold, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    int ret = system(exec_cmd);
    gprint(fg(fmt::color::yellow) | fmt::emphasis::bold, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

    return ret;
}

// Run a permanent executable, return the return code of the executable or 1 if the executable does not exist.
int run_permanent(const Settings &settings, const std::string &name) {
    // rcc paths
    Paths &paths = Paths::get_instance();

    Path cpp_path, bin_path, desc_path;
    paths.get_src_bin_full_path_permanent(name, cpp_path, bin_path, desc_path);

    if (!bin_path.exists()) {
        // TODO: suggest similar names
        print(stderr, "Error: permanent '{}' does not exist\n", name);
        return 1;
    }

    // Run the executable
    return run_bin(settings, cpp_path, bin_path);
}

// List all permanent executables, return 1 on error.
int list_permanent() {
    // rcc paths
    Paths &paths = Paths::get_instance();

    try {
        auto files = find_files(paths.get_sub_permanent_dir().get_path(), {".desc"});
        gprint("Found {} files\n", files.size());

        for (const auto &file : files) {
            std::string content = Path(file).read_file();

            Path cpp_path, bin_path, desc_path;
            paths.get_src_bin_full_path_permanent(file.stem(), cpp_path, bin_path, desc_path);
            // Show in red if the binary doesn't exist (compilation failed or has been deleted)
            auto color = bin_path.exists() ? fmt::terminal_color::green : fmt::terminal_color::red;
            print("{}: {}\n", fmt::styled(file.stem(), fg(color)), content);
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

// Remove permanent files, return 0 if all files were removed successfully, 1 otherwise.
int remove_permanents(const Settings &settings) {
    // rcc paths
    Paths &paths = Paths::get_instance();

    int ret = 0;
    size_t num_removed = 0;
    for (const auto &permanent : settings.get_remove_permanent()) {
        Path cpp_path, bin_path, desc_path;
        paths.get_src_bin_full_path_permanent(permanent, cpp_path, bin_path, desc_path);
        // TODO: does remove throw exception if file does not exist? If so, catch it.
        bool success = cpp_path.remove();
        success |= bin_path.remove();
        success |= desc_path.remove();
        if (success) {
            ++num_removed;
            gprint("Removed '{}'\n", permanent);
        } else {
            ret = 1;
            print(stderr, "Permanent '{}' does not exist!\n", permanent);
        }
    }

    print("Removed {} permanent(s), {} failed.\n", num_removed, settings.get_remove_permanent().size() - num_removed);

    return ret;
}

enum class TryStatus { SUCCESS, COMPILE_FAILED, ERROR };

struct TryResult {
    TryStatus status;
    int return_code;
};

// Silent mode: no output of compiler errors, and no output after the compilation failed.
TryResult try_code(Settings &settings, const string &code, bool silent = false) {
    // rcc paths
    Paths &paths = Paths::get_instance();

    // the compiler
    const string compiler = settings.get_compiler();

    const string cxxflags = settings.get_cxxflags_as_string();
    const string additional_flags = settings.get_additional_flags_as_string();
    const string additional_includes = settings.get_additional_includes_as_string();
    const string above_main = settings.get_above_main_as_string();
    const string functions = settings.get_functions_as_string();
    const string additional_sources = settings.get_additional_sources_as_string();

    // the output cpp code and executable file's full paths
    Path cpp_path, bin_path, desc_path;
    bool desc_written = false;

    if (settings.get_permanent().empty()) {
        // The string to hash, which determines the output file name.
        // It is used to determine if we need to recompile the code or not.
        const string to_hash = code + "a" + compiler + "b" + cxxflags + "a" + additional_flags + "c" +
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
    auto cs = std::shared_ptr<compiler_support>(new_compiler_support(compiler, settings));

    // The hash of this string will be written into the cpp file, so that if one of the fields change, we can detect it
    // and recompile the code. This is an insurance in case the first hash collides.
    //* So in theory, if this program somehow runs the wrong binary, then the two different inputs must have the same
    //* two hashes, and the same code, includes, above main, and functions, since these fields will go into the cpp file
    //* as well, and as what they were given.
    const string id = compiler + "n" + cxxflags + "i" + additional_flags + "n" + additional_includes + "i" +
                      additional_sources;

    // full c++ code generated by the template and the command line arguments
    const string full_code = cs->gen_code(paths.get_template_file_path(),
                                          settings.get_additional_includes(),
                                          settings.get_above_main(),
                                          settings.get_functions(),
                                          code,
                                          u64_to_string_base64x(fnv1a_64_hash_string(id)));

    /*------------------------------------------------------------------------*/
    // * Compile If Needed

    if (settings.get_permanent().empty() && check_if_cached(bin_path, cpp_path, full_code)) {
        // Cached, skip the compiling process, run the executable directly
        gprint(fg(fmt::color::green), "Running cached binary\n");
    } else {
        if (!settings.get_permanent().empty()) {
            // TODO: confirm overwrite, add option -f, --force
        }

        // Write c++ code to the cpp file
        cpp_path.write_file(full_code);

        if (!compile_code(settings, bin_path, cpp_path, cxxflags, additional_flags, cs, silent)) {
            return {TryStatus::COMPILE_FAILED, 1}; // Compile failed
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

        return {TryStatus::SUCCESS, 0};
    }

    /*------------------------------------------------------------------------*/
    // * Run the Executable

    int ret = run_bin(settings, cpp_path, bin_path);

    return {TryStatus::SUCCESS, ret};
}

// The main function of rcc.
// Convenient for testing.
int rcc_main(int argc, char **argv) {
    // TODO: --error-exitcode=<number> exit code to return if errors found [0=disable]
    // TODO: add -q, --quiet, or --silent flag to suppress output
    // TODO: add the fmt, ghc libraries
    // TODO: no color for non-tty
    // TODO: add version info
    // TODO: add option, --permanent NAME, make it permanent, give it a name, save as json file, or source and binary?
    // TODO: add option, --run-permanent NAME, run the permanent one
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

    // Reset colors at exit to avoid terminal issues after program termination
    std::atexit([]() { std::cout << rang::style::reset; });

    // Handle signals gracefully
    register_signal_handler();

    // Parse arguments and set up settings
    Settings settings;
    int result;
    if ((result = settings.parse_argv(argc, argv)) != 0) {
        return result;
    }

    // Print the settings
    gstmt(settings.debug_print());

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
        return list_permanent();
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
        return 0;
    }

    auto &codes = settings.get_codes();

    // If the last code snippet doesn't end with ';' or '}', then, wrap it in
    // 'cout << ... << endl;' and try to compile and run it.
    // This is for convenience, e.g. rcc '2+3*5'.
    if (codes.size() > 0) {
        auto &last_code = codes.back();
        if (last_code.length() > 0 && last_code.back() != ';' && last_code.back() != '}') {
            string code;

            for (size_t i = 0; i < codes.size() - 1; i++) {
                code.append(codes[i]);
            }
            code.append("cout << (" + last_code + ") << endl;");

            gprint(fg(fmt::color::dodger_blue) | fmt::emphasis::bold,
                   ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            gprint("Tring to wrap code with 'cout << ... << endl;'\n");
            auto try_result = try_code(settings, code, true); // silent mode
            gprint(fg(fmt::color::dodger_blue) | fmt::emphasis::bold,
                   "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            if (try_result.status == TryStatus::SUCCESS) {
                return try_result.return_code;
            }
        }
    }

    // command line c++ code
    const string code = settings.get_codes_as_string();

    // Try to compile and run the unwrapped code
    auto try_result = try_code(settings, code);

    // TODO: optimize the return code
    return try_result.return_code;
}

int main(int argc, char **argv) {
    return rcc_main(argc, argv);
}
