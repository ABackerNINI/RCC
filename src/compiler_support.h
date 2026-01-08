#ifndef __RCC_COMPILER_SUPPORT_H__
#define __RCC_COMPILER_SUPPORT_H__

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

    // Virtual destructor to allow deletion of derived class objects through base class pointers
    virtual ~compiler_support() = default;

    // Get the name of the compiler.
    const std::string &get_compiler_name() const { return compiler_name; }

    // Assemble c++ code using the template file and command line arguments.
    // This function will replace the placeholders in the template file with the given arguments.
    // This function may be overridden by subclasses to provide specific behavior for different compilers.
    virtual std::string gen_code(const Path &template_filename,
                                 const std::vector<std::string> &includes,
                                 const std::vector<std::string> &above_main,
                                 const std::vector<std::string> &functions,
                                 const std::string &commandline_code,
                                 const std::string &identifier) const;

    // Generate the compile command to compile the given sources into a binary using that compiler.
    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const = 0;

  protected:
    static size_t safe_replace(std::string &str, size_t pos, const std::string &from, const std::string &to);
    std::string gen_additional_includes(const std::vector<std::string> &additional_includes) const;

  protected:
    std::string compiler_name; // the name of the compiler, e.g., "g++"
    const Settings &settings; // reference to the settings object so that the compiler can access all settings
};

// Subclass for Linux g++ compiler.
class linux_gcc : public compiler_support {
  public:
    linux_gcc(const Settings &settings) : compiler_support("g++", settings) {}

    // Virtual destructor to allow proper cleanup of derived classes.
    virtual ~linux_gcc() = default;

    // Generate the compile command for the Linux g++ compiler.
    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const override;
};

// Subclass for Linux clang++ compiler.
class linux_clang : public compiler_support {
  public:
    linux_clang(const Settings &settings) : compiler_support("clang++", settings) {}

    // Virtual destructor to allow proper cleanup of derived classes.
    virtual ~linux_clang() = default;

    // Generate the compile command for the Linux clang++ compiler.
    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const override;
};

// Create a new compiler support object based on the compiler name.
// The returned object should be deleted by the caller.
compiler_support *new_compiler_support(const std::string &compiler_name, const Settings &settings);

} // namespace rcc

#endif // __RCC_COMPILER_SUPPORT_H__
