#ifndef __RCC_H__
#define __RCC_H__

#include "compiler_support.h"
#include "path.h"
#include "settings.h"
#include <string>

namespace rcc {

class RCC {
  public:
    // The main function of rcc.
    // Convenient for testing.
    int rcc_main(const Settings &settings);

    // Generate the execution command with the binary path and command line arguments.
    static std::string gen_exec_cmd(const Settings &settings, const Path &bin_path);

    // Run the binary executable, return the exit status of the executable, or 1 on error.
    static int run_bin(const Settings &settings, const Path &cpp_path, const Path &bin_path);

    // Compile the file.
    // Silent mode: no output of compiler errors, and no output after the compilation failed.
    static bool compile_file(const Settings &settings,
                             const Path &cpp_path,
                             const Path &bin_path,
                             compiler_support &cs,
                             bool silent);

    // Generate hash for the output filename.
    static std::string gen_first_hash_filename(const Settings &settings, const std::string &code);

    // Generate hash for the identifier.
    static std::string gen_second_hash_identifier(const Settings &settings);

  private:
    // Delete old cached files with probability of 1/256.
    pid_t random_clean_cache();

    // Clean up all cached sources and binaries.
    int clean_cache();

    // Suggest a similar permanent, return empty string if not match found.
    std::string suggest_similar_permanent(const std::string &name);

    // Run a permanent executable, return the return code of the executable or 1 if the executable does not exist.
    int run_permanent(const Settings &settings, const std::string &name);

    // List all permanent executables, return 1 on error.
    int list_permanents(const Settings &settings);

    // Remove file and handle exceptions. Return true if successful, false otherwise.
    bool remove_file(Path &p) noexcept;

    // Remove permanent files, return 0 if all files were removed successfully, 1 otherwise.
    int remove_permanents(const Settings &settings);

    struct AutoWrapResult {
        bool tried;
        std::string code;
    };

    // If the last code snippet doesn't end with ';' or '}', then, wrap it in
    // 'cout << ... << endl;' and try to compile and run it.
    // This is for convenience, e.g. rcc '2+3*5'.
    AutoWrapResult gen_auto_wrap_code(const Settings &settings);

    struct TryCodeResult {
        enum TryStatus { SUCCESS, COMPILE_FAILED, ERROR };

        TryStatus status;
        int exit_status;
    };

    // Try to compile and run code for permanent.
    TryCodeResult try_code_permanent(const Settings &settings);

    // Try to compile and run code for normal mode.
    TryCodeResult try_code_normal(const Settings &settings);

    // Silent mode: no output of compiler errors, and no output after the compilation failed.
    TryCodeResult try_code(const Settings &settings);
};

} // namespace rcc

#endif // __RCC_H__
