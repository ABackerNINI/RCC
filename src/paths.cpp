// #include "pch.h"

#include "paths.h"
#include "utils.h"
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

namespace rcc {

// Check if a path exists, if not, exit with error.
static void expect_exists(const fs::path &path) {
    if (!fs::exists(path)) {
        std::cerr << "RCC mandatory file/directory does not exist: " << path << std::endl;
        std::cerr << "Please reinstall RCC." << std::endl;
        exit(1);
    }
}

// Create a directory if it doesn't exist.
static void create_dir_if_not_exists(const fs::path &path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);

            // Set permissions to 755 (rwxr-xr-x)
            fs::perms perms = fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec |
                              fs::perms::others_read | fs::perms::others_exec;
            fs::permissions(path, perms);
        }
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        exit(1);
    }
}

Paths &Paths::get_instance() {
    static Paths instance;
    return instance;
}

Paths::Paths() {
    // Save cwd
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("getcwd");
        exit(1);
    }

    this->cwd = std::string(cwd);

    validate_cache_dir();
}

// Check the cache directory of rcc.
void Paths::validate_cache_dir() {
    // Get rcc cache directory, default is $HOME/.cache/rcc
    cache_dir = RCC_CACHE_DIR;
    if (cache_dir.string().empty()) {
        const char *HOME = getenv("HOME");
        if (HOME == NULL) {
            std::cerr << "Can't get $HOME" << std::endl;
            exit(1);
        }
        cache_dir = fs::path(HOME) / ".cache/rcc";
    }

    sub_cache_dir = cache_dir / SUB_DIR_CACHE;
    sub_templates_dir = cache_dir / SUB_DIR_TEMPLATES;
    sub_permanent_dir = cache_dir / SUB_DIR_PERMANENT;
    sub_clang_pch_test_cache_dir = cache_dir / SUB_DIR_CLANG_PCH_TEST;

    template_path = this->sub_templates_dir / "rcc_template.cpp";
    template_header_path = this->sub_templates_dir / "rcc_template.hpp";
    template_pch_path = this->sub_templates_dir / "rcc_template.hpp.gch";

    // Check if the mandatory files or directories exist, if not, exit
    expect_exists(cache_dir.get_path());
    expect_exists(sub_templates_dir.get_path());
    expect_exists(template_path.get_path());
    expect_exists(template_header_path.get_path());

    // Check if the non-mandatory directories exist, if not, create them
    create_dir_if_not_exists(sub_cache_dir.get_path());
    create_dir_if_not_exists(sub_permanent_dir.get_path());
    create_dir_if_not_exists(sub_clang_pch_test_cache_dir.get_path());
}

void Paths::get_src_bin_full_path(const std::string &code_for_hash, Path &src_path, Path &bin_path) const {
    const uint64_t hash = fnv1a_64_hash_string(code_for_hash);

    // Max value of uint64_t is 18446744073709551615, 20 digits,
    // so we make the length of hash_str to 20 so that all the filenames of
    // files generated will be the same length
    // string hash_str = std::to_string(hash);
    // hash_str = string(20 - hash_str.length(), '0') + hash_str;

    std::string hash_str = u64_to_string_base64x(hash);

    // write temporary c++ code in this file
    const std::string out_cpp_name = hash_str + ".cpp";
    // compile output file
    const std::string out_bin_name = hash_str + ".bin";

    // the source code full path
    src_path = cache_dir / SUB_DIR_CACHE / out_cpp_name;

    // the executable full path
    bin_path = cache_dir / SUB_DIR_CACHE / out_bin_name;
}

void Paths::get_src_bin_full_path_permanent(const std::string &name,
                                            Path &src_path,
                                            Path &bin_path,
                                            Path &desc_path) const {
    src_path = cache_dir / sub_permanent_dir / (name + ".cpp");
    bin_path = cache_dir / sub_permanent_dir / (name + ".bin");
    desc_path = cache_dir / sub_permanent_dir / (name + ".desc");
}

} // namespace rcc
