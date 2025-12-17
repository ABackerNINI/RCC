#ifndef __COMPILER_SUPPORT_H__
#define __COMPILER_SUPPORT_H__

#include <string>
#include <vector>

#include "paths.h"

namespace rcc {

class compiler_support {
  public:
    compiler_support(const std::string &compiler_name) : compiler_name(compiler_name) {}
    virtual ~compiler_support() = default;

    const std::string &get_compiler_name() const { return compiler_name; }

    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const = 0;

  private:
    std::string compiler_name;
};

class linux_gcc : public compiler_support {
  public:
    linux_gcc() : compiler_support("g++") {}
    virtual ~linux_gcc() = default;

    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const override;
};

class linux_clang : public compiler_support {
  public:
    linux_clang() : compiler_support("clang++") {}
    virtual ~linux_clang() = default;

    virtual std::string get_compile_command(const std::vector<Path> &sources,
                                            const Path &bin_path,
                                            const std::string &cxxflags,
                                            const std::string &additional_flags) const override;
};

} // namespace rcc

#endif // __COMPILER_SUPPORT_H__
