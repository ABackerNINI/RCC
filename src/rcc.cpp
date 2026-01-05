#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include <cstring>
#include <ctime>

#include <iostream>

#include "compiler_support.h"
#include "paths.h"
#include "settings.h"

#include "libs/rang.hpp"

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

// Assemble c++ code using the template file and command line arguments.
string gen_code(const Path &template_filename, const string &commandline_code) {
    string temp = template_filename.read_file();
    // The template file should be checked during installation
    // so do not check it here
    string code = temp.replace(temp.find("$code"), 5, commandline_code);

    return code;
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

int clean_cache() {
    //! Caution: rm command
    string rm_cmd = "rm -f " + Paths::get_instance().get_sub_cache_dir().join("/*").get_path();
    return system(rm_cmd.c_str());
}

#include <csignal>

void signal_handler(int s) {
    (void)s;
    std::cout << std::endl << rang::style::reset << rang::fg::red << rang::style::bold;
    std::cout << "Control-C detected, exiting..." << rang::style::reset << std::endl;
    std::exit(1); // will call the correct exit func, no unwinding of the stack though
}

// The main function of rcc.
// Convenient for testing.
int rcc_main(int argc, char **argv) {
    // TODO: add option --include, include header file
    // TODO: add option --include_stdc++, include the bits/stdc++.h header file
    // TODO: add option --function, create a function
    // TODO: add option, --debug, show debug messages
    // TODO: add option -c, compile only, return binary's name, run later
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

    std::atexit([]() { std::cout << rang::style::reset; });

    // Nice Control-C
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    Settings settings;
    int result;
    if ((result = settings.parse_argv(argc, argv)) != 0) {
        return result;
    }

    // settings.debug_print();

    // Seed the random number generator
    //? Why time() + getpid()?
    //* If we only use time() as the seed, rcc may run in one second multiple
    //* times, and get the same seed.
    srand((unsigned int)time(NULL) + (unsigned int)getpid());

    // Clean old cached files
    if (settings.clean_cache) { // clean cache manually
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
    const string code = settings.gen_command_line_code();

    // the compiler
    const string compiler = settings.compiler;

    // compile with warning and error messages, but no unused warnings
    const string cxxflags = settings.gen_cxxflags();
    const string additional_flags = settings.gen_additional_flags();

    // the output cpp code and executable file's full paths
    Path cpp_path, bin_path;
    paths.get_src_bin_full_path(code + compiler + cxxflags + additional_flags, cpp_path, bin_path);

    // run the executable from cwd
    const string command_line_args = settings.gen_command_line_args();
    const string run_exec_cmd = bin_path.get_path() + (command_line_args.empty() ? "" : " " + command_line_args);

    // full c++ code generated by the template and the command line arguments
    const string full_code = gen_code(paths.get_template_file_path(), code);

    // Check if cached
    if (bin_path.exists()) {
        const string code_old = cpp_path.read_file();
        if (code_old == full_code) {
            // Cached, skip the compiling process, run the executable directly
            //* Note that the result of this system call is ignored
            // cout << run_exec_str << endl;
            ignore_system(run_exec_cmd);
            return 0;
        }
    }

    // Write c++ code to the temp file
    cpp_path.write_file(full_code);

    /*------------------------------------------------------------------------*/
    // * Compile And Run

    compiler_support *cs = new_compiler_support(compiler);
    const string compile_cmd = cs->get_compile_command({cpp_path}, bin_path, cxxflags, additional_flags);
    delete cs;

    // If compile succeed, run the program from cwd directory,
    // else print the out cpp full path.
    if (system(compile_cmd) == 0) {
        //* Note that the result of this system call is ignored
        ignore_system(run_exec_cmd);
    } else {
        cout << "\n" << cpp_path.get_path() << endl;
        cout << "\n" << rang::fg::red << rang::style::bold << "COMPILATION FAILED!" << rang::style::reset << endl;
        cout << "Compile command: " << rang::style::underline << compile_cmd << rang::style::reset << endl;
        cout << "Execute command: " << rang::style::underline << run_exec_cmd << rang::style::reset << endl;
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) { return rcc_main(argc, argv); }
