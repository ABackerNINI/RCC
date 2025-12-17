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

#define SUB_DIR_CACHE     "cache"
#define SUB_DIR_TEMPLATES "templates"

#define HASH_SEED 13

namespace rcc {
class Path {
  public:
    Path(const std::string &path);
    Path(const char *path);
    Path();

    Path &join(const std::string &other);
    Path &join(const char *other);
    Path &join(const Path &other);

    const std::string &get_path() const;
    std::string get_plain_path() const;

    bool exists() const;
    bool is_dir() const;
    bool is_file() const;

    std::string read_file() const;
    void write_file(const std::string &content) const;

    static std::string check_whitespaces(const std::string &path);

  private:
    std::string path;
};

class Paths {
  public:
    static Paths &get_instance(); // Singleton

    Path get_cwd() const;

    Path get_cache_dir() const;

    Path get_sub_cache_dir() const;
    Path get_sub_templates_dir() const;

    Path get_template_path() const;

    void get_src_bin_full_path(const std::string &code_for_hash, Path &src_path, Path &bin_path) const;

  private:
    Paths();
    void validate_cache_dir();

    static uint64_t hash_string(const std::string &s);

  private:
    Path cwd;
    Path cache_dir;
    Path sub_cache_dir;
    Path sub_templates_dir;
    Path template_path;
};
} // namespace rcc

#endif // __PATHS_H__
