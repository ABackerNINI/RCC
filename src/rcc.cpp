#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <ctime>

#include <iostream>
#include <memory>

#include "libs/debug.h"
#include "libs/rang.hpp"

#include "compiler_support.h"
#include "paths.h"
#include "settings.h"

using namespace std;
using namespace rcc;

// Ignore the result of a function call so that compiler doesn't warn about
// unused return result.
template <typename T> void IGNORE_RESULT(T &&) {}

// Wrapper function of system(const char *);
int system(const string &cmd) {
    // cout << cmd << endl;
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
    std::exit(1); // will call the correct exit func, no unwinding of the stack though
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
            string find_rm_cmd;
            find_rm_cmd += string("find ") + paths.get_sub_cache_dir().get_path();
            find_rm_cmd += string(" -type f");
            find_rm_cmd += string(" \\( -name \"") + RCC_TEMP_SRC_NAME_PREFIX + "*.cpp\"";
            find_rm_cmd += string(" -o -name \"") + RCC_TEMP_BIN_NAME_PREFIX + "*.bin\" \\)";
            //! Caution: rm command
            find_rm_cmd += string(" -atime +30 | xargs rm -f");

            // cout << find_rm_cmd << endl;

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
    const string sub_cache_dir = Paths::get_instance().get_sub_cache_dir().get_path();
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
        cdbg << rang::fg::red << rang::style::bold << "WARNING: hash collided but content does not match!"
             << rang::style::reset << endl;
        cdbg << "Old code: " << rang::fg::blue << code_old << rang::style::reset << endl;
        cdbg << "New code: " << rang::fg::green << full_code << rang::style::reset << endl;
    }
    return false;
}

// Compile the code.
bool compile_code(const Settings &settings,
                  const Path &bin_path,
                  const Path &cpp_path,
                  const string &cxxflags,
                  const string &additional_flags,
                  const string &exec_cmd,
                  shared_ptr<compiler_support> cs) {
    vector<Path> sources = {cpp_path};
    for (auto &src : settings.get_additional_sources()) {
        sources.emplace_back(src);
    }

    const string compile_cmd = cs->get_compile_command(sources, bin_path, cxxflags, additional_flags);

    cdbg << compile_cmd << endl;

    if (system(compile_cmd) != 0) {
        cout << "\n" << cpp_path.get_path() << endl;
        cout << "\n" << rang::fg::red << rang::style::bold << "COMPILATION FAILED!\n" << rang::style::reset << endl;
        cout << rang::fg::yellow << rang::style::bold << "COMPILE COMMAND: " << rang::style::reset << compile_cmd
             << rang::style::reset << endl;
        cout << rang::fg::yellow << rang::style::bold << "EXECUTE COMMAND: " << rang::style::reset << exec_cmd
             << rang::style::reset << endl;
        return false;
    }
    return true;
}

// The main function of rcc.
// Convenient for testing.
int rcc_main(int argc, char **argv) {
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
    dbg_stmt(settings.debug_print());

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

    // No code to compile, just return
    if (!settings.has_code()) {
        return 0;
    }

    // rcc paths
    Paths &paths = Paths::get_instance();

    // command line c++ code
    const string code = settings.get_codes_as_string();

    // the compiler
    const string compiler = settings.get_compiler();

    const string cxxflags = settings.get_cxxflags_as_string();
    const string additional_flags = settings.get_additional_flags_as_string();
    const string additional_includes = settings.get_additional_includes_as_string();
    const string above_main = settings.get_above_main_as_string();
    const string functions = settings.get_functions_as_string();
    const string additional_sources = settings.get_additional_sources_as_string();

    // The string to hash, which determines the output file name.
    // It is used to determine if we need to recompile the code or not.
    const string to_hash = code + "a" + compiler + "b" + cxxflags + "a" + additional_flags + "c" + additional_includes +
                           "k" + above_main + "e" + functions + "r" + additional_sources;

    // the output cpp code and executable file's full paths
    Path cpp_path, bin_path;
    paths.get_src_bin_full_path(to_hash, cpp_path, bin_path);

    // run the executable from cwd
    const string command_line_args = settings.get_cli_args_as_string();
    const string exec_cmd = bin_path.get_path() + (command_line_args.empty() ? "" : " " + command_line_args);

    // the compiler support
    auto cs = shared_ptr<compiler_support>(new_compiler_support(compiler, settings));

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
                                          to_string(Paths::fnv1a_64_hash_string(id)));

    /*------------------------------------------------------------------------*/
    // * Compile If Needed

    if (check_if_cached(bin_path, cpp_path, full_code)) {
        // Cached, skip the compiling process, run the executable directly
        cdbg << "Running cached binary" << endl;
        cdbg << "run: " << exec_cmd << endl;
    } else {
        // Write c++ code to the cpp file
        cpp_path.write_file(full_code);

        if (!compile_code(settings, bin_path, cpp_path, cxxflags, additional_flags, exec_cmd, cs)) {
            return 1; // Compile failed
        }
    }

    /*------------------------------------------------------------------------*/
    // * Run the Executable

    cdbgx << rang::fg::yellow << rang::style::bold << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>" << rang::style::reset << endl;
    int ret = system(exec_cmd);
    cdbgx << rang::fg::yellow << rang::style::bold << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << rang::style::reset << endl;

    return ret;
}

int main(int argc, char **argv) {
    return rcc_main(argc, argv);
}
