#include <ctime>
#include <fstream>
#include <iostream>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#ifndef RCC_COMPILER
// Use this compiler to compile the code.

// The clang++ compiler uses a different strategy for Pre-Compiled Header.
// So use g++ for now.
#define RCC_COMPILER "g++"
#endif

#ifndef RCC_CPP_STD
// Use this standard to compile the code.
#define RCC_CPP_STD "c++11"
#endif

#ifndef RCC_CACHE_DIR
// Store all temporary files in this directory, including auto-generated .cpp
// and .bin files.
#define RCC_CACHE_DIR ""
#endif

#define HASH_SEED 13

#define RCC_TEMP_SRC_NAME_PREFIX "rcc_src_"
#define RCC_TEMP_BIN_NAME_PREFIX "rcc_bin_"
// #define RCC_LOG_NAME "rcc.log"

// Merge all command line arguments together in a string.
// Each argument forms a new line, and indent 4 spaces except the first line.
string merge_args(int argc, char **argv) {
    string s;
    s.reserve(256);
    s += "User code\n    ";
    for (int i = 1; i < argc - 1; ++i) {
        s.append(argv[i]);
    }
    if (argc > 1) {
        s += argv[argc - 1];
    }
    return s;
}

// Read file line by line and put them together in a string.
string read_file(const std::string &filename) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Can't open file: " << filename << endl;
        exit(1);
    }
    string content;
    content.reserve(1024);
    string line;

    while (getline(infile, line)) {
        content += line;
        content.push_back('\n');
    }

    return content;
}

// Return hash of the string.
unsigned long long hash_string(const std::string &s) {
    unsigned long long hash = 0;
    for (const char &c : s) {
        hash = hash * HASH_SEED + (unsigned char)c;
    }
    return hash;
}

// Assemble c++ code using the template file and command line arguments.
string gen_code(const string &template_filename,
                const string &commandline_code) {
    string temp = read_file(template_filename);
    // The template file should be checked during installation
    // so do not check it here
    string code = temp.replace(temp.find("%s"), 2, commandline_code);

    return code;
}

// Write string to a file.
void write_file(const string &filename, const string &content) {
    ofstream outfile(filename);
    if (!outfile) {
        cerr << "Can't open file: " << filename << endl;
        exit(1);
    }

    outfile << content;
}

// Delete old cached files.
//! IMPORTANT: Please make sure that cwd is at the rcc cache directory.
pid_t clean_cache() {
    // 1/256 chances
    //? Why not use the hash value of code as the seed?
    //* The srand() and rand() is fast enough. And the same code has the same
    //* hash value.
    if (rand() % 256 == 0) {
        pid_t pid = fork();
        if (pid == 0) { // in child process
            // find and remove src/bin files whose access time is 30 days ago
            string find_rm_cmd;
            find_rm_cmd += string("find . -atime +30 -name \"") +
                           RCC_TEMP_SRC_NAME_PREFIX + "*.cpp\"";
            find_rm_cmd +=
                string(" -o -name \"") + RCC_TEMP_BIN_NAME_PREFIX + "*.bin\"";
            //! Caution: rm command
            find_rm_cmd += string(" | xargs rm -f");

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

// Ignore the result of a function call so that compiler doesn't warn about
// unused result.
template <typename T> void IGNORE_RESULT(T &&) {}

// Wrapper for system() to ignore return value.
void ignore_system(const char *cmd) {
    // [[maybe_unused]] auto result = system(cmd); // ignore result
    IGNORE_RESULT(system(cmd));
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
    // TODO: add support for clang++
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

    // Save the template cpp file in $HOME/.cache/RCC_TEMP_SRC_NAME_PREFIX
    const char *HOME = getenv("HOME");
    if (HOME == NULL) {
        cerr << "Can't get $HOME" << endl;
        exit(1);
    }

    // Save cwd
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("getcwd");
        exit(1);
    }

    string rcc_cache_dir = RCC_CACHE_DIR;
    if (rcc_cache_dir == "") {
        rcc_cache_dir = string(HOME) + "/.cache/rcc";
    }

    // Create rcc cache directory if not exist
    if (access(rcc_cache_dir.c_str(), F_OK) != 0) {
        if (system(("mkdir -p " + rcc_cache_dir).c_str()) != 0) {
            perror("system:mkdir");
            exit(1);
        }
    }

    /*------------------------------------------------------------------------*/
    // * Working at the cache directory

    // Change to the cache directory
    if (chdir(rcc_cache_dir.c_str()) != 0) {
        perror("chdir");
        exit(1);
    }

    // Clean old cached files
    clean_cache();

    // command line c++ code
    string code = merge_args(argc, argv);

    // cpp template path
    string temp_cpp_path = "rcc_template.cpp";

    unsigned long long hash = hash_string(code);

    string hash_str = to_string(hash);

    // Max value of unsigned long long is 18446744073709551615, 19 digits
    // so we make the length of hash_str to 20 so that all the filenames of
    // files generated will be the same length
    string zeros;
    for (size_t i = 0; i < 20 - hash_str.length(); ++i) {
        zeros.push_back('0');
    }
    hash_str = zeros + hash_str;

    // write temporary c++ code in this file
    string out_cpp_name = RCC_TEMP_SRC_NAME_PREFIX + hash_str + ".cpp";
    // compile output file
    string out_bin_name = RCC_TEMP_BIN_NAME_PREFIX + hash_str + ".bin";
    // the executable full path
    string out_bin_full_path = rcc_cache_dir + "/" + out_bin_name;

    // cd back to the cwd and run the executable from cwd
    string run_exec_cmd = string("cd \"") + cwd + "\"; " + out_bin_full_path;

    // full c++ code generated by the template and the command line arguments
    string full_code = gen_code(temp_cpp_path, code);

    // Check if cached
    if (access(out_bin_name.c_str(), F_OK) == 0) {
        string code_old = read_file(out_cpp_name);
        if (code_old == full_code) {
            // Cached, skip the compiling process, run the executable directly
            //* Note that the result of this system call is ignored
            // cout << run_exec_str << endl;
            ignore_system(run_exec_cmd.c_str());
            return 0;
        }
    }

    // Write c++ code to the temp file
    write_file(out_cpp_name, full_code);

    /*------------------------------------------------------------------------*/
    // * Compile And Run

    // the compiler
    string compiler = RCC_COMPILER;

    //? Should use a C++ standard like "-std=c++11"?
    //* This will be necessary on some lower version compilers. But this will
    //* prevent us using a higher default standard.
    //* The current solution is test C++ standard before making rcc, then use
    //* the highest possible standard to compile the rcc and template header as
    //* well as use it here. See RCC_CPP_STD.

    // compile with warning and error messages, but no unused warnings
    string compile_cmd = compiler + " " + RCC_CPP_STD +
                         " -W -Wall"
                         " -Wno-unused-variable"
                         " -Wno-unused-parameter"
                         " -Wno-unused-function"
                         " -o " +
                         out_bin_name + " " + out_cpp_name;
    // compile without warning and error messages will be ignored
    // string compile_without_warning_log_cmd =
    //   compiler + " -o " + out_bin_name + " " + out_cpp_name + " 2>/dev/null";

    // If compile succeed, run the program from cwd directory,
    // else print the out cpp full path.
    if (system(compile_cmd.c_str()) == 0) {
        //* Note that the result of this system call is ignored
        ignore_system(run_exec_cmd.c_str());
    } else {
        cout << "\n" << rcc_cache_dir << "/" << out_cpp_name << endl;
    }

    return 0;
}
