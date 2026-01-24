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

  private:
    // Delete old cached files with probability of 1/256.
    pid_t random_clean_cache();

    // Clean up all cached sources and binaries.
    int clean_cache();

    // Check if the binary is cached and the content matches.
    //* The file hash may collide, so we need to check the content as well.
    bool check_if_cached(const Path &bin_path, const Path &cpp_path, const std::string &full_code);

    // Generate the execution command with the binary path and command line arguments.
    std::string gen_exec_cmd(const Settings &settings, const Path &bin_path);

    // Compile the code.
    bool compile_code(const Settings &settings,
                      const Path &bin_path,
                      const Path &cpp_path,
                      const compiler_support &cs,
                      bool silent = false);

    // Run the binary executable, return the return code of the executable.
    int run_bin(const Settings &settings, const Path &cpp_path, const Path &bin_path);

    // Suggest a similar permanent, return empty string if not match found.
    std::string suggest_similar_permanent(const std::string &name);

    // Run a permanent executable, return the return code of the executable or 1 if the executable does not exist.
    int run_permanent(const Settings &settings, const std::string &name);

    // List all permanent executables, return 1 on error.
    int list_permanent(const Settings &settings);

    // Remove file and handle exceptions. Return true if successful, false otherwise.
    bool remove_file(Path &p) noexcept;

    // Remove permanent files, return 0 if all files were removed successfully, 1 otherwise.
    int remove_permanents(const Settings &settings);

    struct TryCodeResult {
        enum TryStatus { SUCCESS, COMPILE_FAILED, ERROR };

        TryStatus status;
        int exit_status;
    };

    // Silent mode: no output of compiler errors, and no output after the compilation failed.
    TryCodeResult try_code(const Settings &settings, const std::string &code, bool silent = false);

    struct AutoWrapResult {
        bool tried;
        TryCodeResult try_result;

        // For C++11 compatibility
        AutoWrapResult(bool tried = false, TryCodeResult try_result = TryCodeResult())
            : tried(tried), try_result(try_result) {}
    };

    AutoWrapResult auto_wrap(const Settings &settings);
};

} // namespace rcc

#endif // __RCC_H__
