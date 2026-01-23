#ifndef __RCC_PATHS_H__
#define __RCC_PATHS_H__

#include "path.h"
#include <string>

#ifndef RCC_CACHE_DIR
    // Store all temporary files in this directory, including auto-generated .cpp
    // and .bin files.
    #define RCC_CACHE_DIR ""
#endif

// #define RCC_TEMP_SRC_NAME_PREFIX "c"
// #define RCC_TEMP_BIN_NAME_PREFIX "c"
// #define RCC_LOG_NAME "rcc.log"

#define SUB_DIR_CACHE "cache"
#define SUB_DIR_TEMPLATES "templates"
#define SUB_DIR_PERMANENT "permanent"
#define SUB_DIR_CLANG_PCH_TEST "templates/clang_pch_test_cache"

namespace rcc {

// Paths that are used by rcc.
// This class is a singleton.
class Paths {
  public:
    // Get the singleton instance of Paths.
    static Paths &get_instance();

    // Get the current working directory.
    const Path &get_cwd() const { return cwd; }

    // Get the cache root directory. This is where the templates and compiled binaries are stored.
    // Usually ~/.cache/rcc.
    const Path &get_cache_dir() const { return cache_dir; }

    // Get the sub cache directory. This is where the compiled binaries are stored.
    // Usually ~/.cache/rcc/cache.
    const Path &get_sub_cache_dir() const { return sub_cache_dir; }

    // Get the sub templates directory. This is where the templates are stored.
    // Usually ~/.cache/rcc/templates.
    const Path &get_sub_templates_dir() const { return sub_templates_dir; }

    // Get the sub permanent directory. This is where the permanent files are stored.
    // Usually ~/.cache/rcc/permanent.
    const Path &get_sub_permanent_dir() const { return sub_permanent_dir; }

    // Get the sub clang pch test cache directory. This is where the clang pch test cache files are stored.
    // Usually ~/.cache/rcc/templates/clang_pch_test_cache.
    const Path &get_sub_clang_pch_test_cache_dir() const { return sub_clang_pch_test_cache_dir; }

    // Get the template cpp file path. User code is written to this file.
    // Usually ~/.cache/rcc/templates/rcc_template.cpp.
    const Path &get_template_file_path() const { return template_path; }

    // Get the template header file path. This file contains commonly used includes and some definitions.
    // Usually ~/.cache/rcc/templates/rcc_template.hpp.
    const Path &get_template_header_path() const { return template_header_path; }

    // Get the template pch file/directory path which contains the precompiled headers.
    // Usually ~/.cache/rcc/templates/rcc_template.hpp.pch.
    const Path &get_template_pch_path() const { return template_pch_path; }

    // Get the full path of the output cpp file and binary file for a given piece of code.
    // The filenames are based on the hash of the code.
    void get_src_bin_full_path(const std::string &code_for_hash, Path &src_path, Path &bin_path) const;

    // Get the full path of the given permanent code name.
    void get_src_bin_full_path_permanent(const std::string &name,
                                         Path &src_path,
                                         Path &bin_path,
                                         Path &desc_path) const;

  private:
    // Private constructor to prevent instantiation.
    Paths();

    // Validate the root cache directory to ensure that everything is set up correctly.
    void validate_cache_dir();

  private:
    Path cwd;
    Path cache_dir;
    Path sub_cache_dir;
    Path sub_templates_dir;
    Path sub_permanent_dir;
    Path sub_clang_pch_test_cache_dir;
    Path template_path;
    Path template_header_path;
    Path template_pch_path;
};
} // namespace rcc

#endif // __RCC_PATHS_H__
