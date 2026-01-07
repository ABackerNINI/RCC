#ifndef __PATHS_H__
#define __PATHS_H__

#include <cstdint>

#include <string>

#ifndef RCC_CACHE_DIR
// Store all temporary files in this directory, including auto-generated .cpp
// and .bin files.
#define RCC_CACHE_DIR ""
#endif

#define RCC_TEMP_SRC_NAME_PREFIX "rcc_src_"
#define RCC_TEMP_BIN_NAME_PREFIX "rcc_bin_"
// #define RCC_LOG_NAME "rcc.log"

#define SUB_DIR_CACHE "cache"
#define SUB_DIR_TEMPLATES "templates"

namespace rcc {

// Represent a path in the filesystem. Maybe use std::filesystem in C++17?
//* Currently, this supports only Linux.
class Path {
  public:
    Path(const std::string &path);
    Path(const char *path);
    Path();

    // Join a path with another path.
    Path &join(const std::string &other);

    // Join a path with another path.
    Path &join(const char *other);

    // Join a path with another path.
    Path &join(const Path &other);

    // Get the path as a string.
    const std::string &get_path() const;

    // Get the path as a string, without any quotations or trailing slashes.
    std::string get_plain_path() const;

    // Check if the path exists.
    bool exists() const;

    // Check if the path is a directory.
    bool is_dir() const;

    // Check if the path is a file.
    bool is_file() const;

    // Read the file at the path. Return the contents as a string.
    std::string read_file() const;

    // Write the given content to the file at the path.
    void write_file(const std::string &content) const;

  private:
    // Return the path with quotes around it if it contains spaces.
    static std::string check_whitespaces(const std::string &path);

  private:
    std::string path;
};

// Paths that are used by rcc.
// This class is a singleton.
class Paths {
  public:
    // Get the singleton instance of Paths.
    static Paths &get_instance();

    // Get the current working directory.
    Path get_cwd() const;

    // Get the cache root directory. This is where the templates and compiled binaries are stored.
    // Usually ~/.cache/rcc.
    Path get_cache_dir() const;

    // Get the sub cache directory. This is where the compiled binaries are stored.
    // Usually ~/.cache/rcc/cache.
    Path get_sub_cache_dir() const;

    // Get the sub templates directory. This is where the templates are stored.
    // Usually ~/.cache/rcc/templates.
    Path get_sub_templates_dir() const;

    // Get the template cpp file path. User code is written to this file.
    Path get_template_file_path() const;

    // Get the full path of the output cpp file and binary file for a given piece of code.
    // The filenames are based on the hash of the code.
    void get_src_bin_full_path(const std::string &code_for_hash, Path &src_path, Path &bin_path) const;

    // Hash a string using the FNV-1a algorithm. This is used to generate unique filenames
    // for the source and binary files.
    static uint64_t fnv1a_64_hash_string(const std::string &str);

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
    Path template_path;
};
} // namespace rcc

#endif // __PATHS_H__
