#ifndef __COMPILER_SUPPORT_H__
#define __COMPILER_SUPPORT_H__

#include <string>
#include <vector>

#include "paths.h"
#include "settings.h"

namespace rcc {

// Abstract base class for compiler support.
// Each subclass implements the get_compile_command() method for a specific compiler.
// The get_compile_command() method returns a string that can be executed to compile
// the given sources into a binary using that compiler.
class compiler_support {
  public:
    compiler_support(const std::string &compiler_name, const Settings &settings)
        : compiler_name(compiler_name), settings(settings) {}
    virtual ~compiler_support() = default;

    const std::string &get_compiler_name() const { return compiler_name; }

    // Assemble c++ code using the template file and command line arguments.
    virtual std::string gen_code(const Path &template_filename,
                                 const std::vector<std::string> &includes,
                                 const std::vector<std::string> &functions,
                                 const std::string &commandline_code) const = 0;

    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const = 0;

  protected:
    std::string gen_additional_includes(const std::vector<std::string> &additional_includes) const;

  protected:
    std::string compiler_name;
    const Settings &settings;
};

// Subclass for Linux g++ compiler.
class linux_gcc : public compiler_support {
  public:
    linux_gcc(const Settings &settings) : compiler_support("g++", settings) {}
    virtual ~linux_gcc() = default;

    virtual std::string gen_code(const Path &template_filename,
                                 const std::vector<std::string> &includes,
                                 const std::vector<std::string> &functions,
                                 const std::string &commandline_code) const override;

    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const override;
};

// Subclass for Linux clang++ compiler.
class linux_clang : public compiler_support {
  public:
    linux_clang(const Settings &settings) : compiler_support("clang++", settings) {}
    virtual ~linux_clang() = default;

    virtual std::string gen_code(const Path &template_filename,
                                 const std::vector<std::string> &includes,
                                 const std::vector<std::string> &functions,
                                 const std::string &commandline_code) const override;

    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const override;
};

compiler_support *new_compiler_support(const std::string &compiler_name, const Settings &settings);

} // namespace rcc

#endif // __COMPILER_SUPPORT_H__
