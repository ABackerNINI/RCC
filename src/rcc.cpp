#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include <cstring>
#include <ctime>

#include <iostream>

#include "compiler_support.h"
#include "paths.h"

using namespace std;
using namespace rcc;

#ifndef RCC_COMPILER
// Use this compiler to compile the code.
//* The clang++ compiler uses a different strategy for Pre-Compiled Header.
//* So use g++ for now.
#define RCC_COMPILER "g++"
#endif

#ifndef RCC_CPP_STD
// Use this standard to compile the code, at least c++11.
#define RCC_CPP_STD "c++11"
#endif

// Ignore the result of a function call so that compiler doesn't warn about
// unused result.
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

// Merge all command line arguments together in a string.
// Each argument forms a new line, and indent 4 spaces except the first line.
string merge_args(int argc, char **argv) {
    string s;
    s.reserve(256);
    s += "User code\n    ";
    // If there is only one argument and it doesn't end with ';', wrap it in
    // 'cout << ... << endl;' directly. This is for convenience,
    // e.g. rcc '2+3*5'
    if (argc == 2 && strlen(argv[1]) > 0 && argv[1][strlen(argv[1]) - 1] != ';') {
        s += "cout << " + string(argv[1]) + " << endl;";
    } else {
        // Otherwise, merge all arguments together without any modification.
        for (int i = 1; i < argc; ++i) {
            s.append(argv[i]);
        }
    }
    return s;
}

// Assemble c++ code using the template file and command line arguments.
string gen_code(const Path &template_filename, const string &commandline_code) {
    string temp = template_filename.read_file();
    // The template file should be checked during installation
    // so do not check it here
    string code = temp.replace(temp.find("%s"), 2, commandline_code);

    return code;
}

// Delete old cached files.
pid_t clean_cache() {
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

int main(int argc, char **argv) {
    // TODO: rcc_debug: show debug messages
    // TODO: rcc_print, rcc_print_pretty: print c++ code only
    // TODO: rcc_gen: compile only, return binary's name, run later
    // TODO: rcc_clean: clean cached files manually
    // TODO: rcc_argv: pass argv directly to rcc, e.g. rcc_argv a b c | rcc ''
    // TODO: add option, -i or -l, add lib
    // TODO: add option, --debug
    // TODO: add option cat, cat last generated code
    // TODO: add option --print only
    // TODO: add option --pretty
    // TODO: add option -c, compile only, return binary's name, run later
    // TODO: add option -c, wrap the code in a "cout" and "endl"
    // TODO: add option --arg, run with arguments
    // TODO: add option --clean, clean cached files manually
    // TODO: add support for Windows
    // TODO: make install_local: install it at the current directory
    // TODO: auto detect compiler
    // TODO: check template files during make install
    // TODO: store in .local, reproduce install if .cache/rcc files been removed
    // TODO: add a readme in .cache/rcc
    // TODO: add version and help messages
    // TODO: boost with multi-thread
    //? TODO: add option --stdin, read input from stdin instead of arguments

    // Seed the random number generator
    //? Why time() + getpid()?
    //* If we only use time() as the seed, rcc may run in one second multiple
    //* times, and get the same seed.
    srand((unsigned int)time(NULL) + (unsigned int)getpid());

    // Clean old cached files
    clean_cache();

    // rcc paths
    Paths &paths = Paths::get_instance();

    // command line c++ code
    string code = merge_args(argc, argv);

    // the output cpp code and executable file's full paths
    Path cpp_path, bin_path;
    paths.get_src_bin_full_path(code, cpp_path, bin_path);

    // run the executable from cwd
    const string run_exec_cmd = bin_path.get_path();

    // full c++ code generated by the template and the command line arguments
    const string full_code = gen_code(paths.get_template_path(), code);

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

    // the compiler
    const string compiler = RCC_COMPILER;

    //? Should use a C++ standard like "-std=c++11"?
    //* This will be necessary on some lower version compilers. But this will
    //* prevent us using a higher default standard.
    //* The current solution is test C++ standard before making rcc, then use
    //* the highest possible standard to compile the rcc and template header as
    //* well as use it here. See RCC_CPP_STD.

    compiler_support *cs = NULL;
    if (compiler == "g++") {
        cs = new linux_gcc();
    } else if (compiler == "clang++") {
        cs = new linux_clang();
    } else {
        cerr << "Unsupported compiler: " + compiler + "\n";
        exit(EXIT_FAILURE);
    }

    // compile with warning and error messages, but no unused warnings
    const string cxxflags = string(RCC_CPP_STD) + " -Wno-unused-variable"
                                                  " -Wno-unused-parameter"
                                                  " -Wno-unused-function";
    const string additional_flags = "";
    const string compile_cmd = cs->get_compile_command({cpp_path}, bin_path, cxxflags, additional_flags);

    delete cs;

    // If compile succeed, run the program from cwd directory,
    // else print the out cpp full path.
    if (system(compile_cmd) == 0) {
        //* Note that the result of this system call is ignored
        ignore_system(run_exec_cmd);
    } else {
        cout << "\n" << cpp_path.get_path() << endl;
        cout << "\n" << "Compilation failed!" << endl;
        cout << "Try to compile with: " << compile_cmd << endl;
        cout << "Try to run with: " << run_exec_cmd << endl;
    }

    return 0;
}
