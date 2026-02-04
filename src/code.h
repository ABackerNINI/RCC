#ifndef __RCC_CODE_H__
#define __RCC_CODE_H__

#include "compiler_support.h"
#include "paths.h"
#include "settings.h"
#include <string>

namespace rcc {

class RCCode {
  public:
    RCCode(const Settings &settings,
           const Paths &paths,
           const std::string &identifier,
           compiler_support &cs,
           const std::string &code,
           const std::string &code_name)
        : settings(settings), paths(paths), identifier(identifier), cs(cs), code(code), code_name(code_name) {}

    // Init the output cpp and bin paths.
    // This requires the full_code to be generated first.
    void init_cpp_bin_paths();

    // Check if the binary is cached and the content matches.
    //* The file hash may collide, so we need to check the content as well.
    bool is_cached();

    // Write the full code to the cpp file and compile it.
    // This requires the full_code to be generated first.
    // Silent mode: no output of compiler errors, and no output after the compilation failed.
    bool compile(bool silent);

    // Run the binary executable, return the exit status of the executable, or 1 on error.
    int run_bin();

  private:
    // Generate the full code with the given code and settings.
    void gen_full_code();

  protected:
    const Settings &settings;
    const Paths &paths;
    const std::string identifier;
    compiler_support &cs;
    std::string code;
    std::string code_name;
    std::string full_code;
    Path cpp_path;
    Path bin_path;
    bool full_code_generated{false};
};

class RCCodePermanent : public RCCode {
  public:
    RCCodePermanent(const Settings &settings,
                    const Paths &paths,
                    const std::string &identifier,
                    compiler_support &cs,
                    const std::string &code,
                    const std::string &code_name)
        : RCCode(settings, paths, identifier, cs, code, code_name) {}

    // Init the output cpp and bin paths as well as the desc file path.
    void init_cpp_bin_paths() {
        paths.get_src_bin_full_path_permanent(settings.get_permanent(), cpp_path, bin_path, desc_path);
    }

  protected:
    Path desc_path;
};

} // namespace rcc

#endif // __RCC_CODE_H__
