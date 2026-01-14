#include "pch.h"

#include "paths.h"
#include "utils.h"

namespace rcc {

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

    // TODO: rename to validate_installation()
    validate_cache_dir();

    // TODO: move into validate_installation()
    this->template_path = sub_templates_dir / "rcc_template.cpp";
    this->template_header_path = sub_templates_dir / "rcc_template.hpp";
}

// Check the cache directory of rcc.
void Paths::validate_cache_dir() {
    const char *HOME = getenv("HOME");
    if (HOME == NULL) {
        cerr << "Can't get $HOME" << endl;
        exit(1);
    }

    // Get rcc cache directory, default is $HOME/.cache/rcc
    string cache_dir = RCC_CACHE_DIR;
    if (cache_dir == "") {
        cache_dir = string(HOME) + "/.cache/rcc";
    }

    const string sub_cache_dir = cache_dir + "/" + SUB_DIR_CACHE;
    const string sub_templates_dir = cache_dir + "/" + SUB_DIR_TEMPLATES;
    const string sub_permanent_dir = cache_dir + "/" + SUB_DIR_PERMANENT;

    // TODO: reduce duplicated code

    // Check if the cache directory exists, if not, exit
    if (access(cache_dir.c_str(), F_OK) != 0) {
        cerr << "Cache directory does not exist: " << cache_dir << endl;
        cerr << "Please reinstall rcc." << endl;
        exit(1);
    }

    // Check if the templates sub-directory exists, if not, exit
    if (access(sub_templates_dir.c_str(), F_OK) != 0) {
        cerr << "Templates directory does not exist: " << cache_dir + "/" + SUB_DIR_TEMPLATES << endl;
        cerr << "Please reinstall rcc." << endl;
        exit(1);
    }

    // Check if the cache sub-directory exists, if not, create it
    if (access(sub_cache_dir.c_str(), F_OK) != 0) {
        if (mkdir(sub_cache_dir.c_str(), 0755) != 0) {
            cerr << "Failed to create cache directory: " << sub_cache_dir << endl;
            exit(1);
        }
    }

    // Check if the permanent sub-directory exists, if not, create it
    if (access(sub_permanent_dir.c_str(), F_OK) != 0) {
        if (mkdir(sub_permanent_dir.c_str(), 0755) != 0) {
            cerr << "Failed to create cache directory: " << sub_permanent_dir << endl;
            exit(1);
        }
    }

    this->cache_dir = cache_dir;
    this->sub_cache_dir = sub_cache_dir;
    this->sub_templates_dir = sub_templates_dir;
    this->sub_permanent_dir = sub_permanent_dir;
}

Path Paths::get_cache_dir() const {
    return this->cache_dir;
}

Path Paths::get_sub_cache_dir() const {
    return this->sub_cache_dir;
}

Path Paths::get_sub_templates_dir() const {
    return this->sub_templates_dir;
}

Path Paths::get_sub_permanent_dir() const {
    return this->sub_permanent_dir;
}

Path Paths::get_template_file_path() const {
    return this->template_path;
}

Path Paths::get_template_header_path() const {
    return this->template_header_path;
}

void Paths::get_src_bin_full_path(const std::string &code_for_hash, Path &src_path, Path &bin_path) const {
    const uint64_t hash = fnv1a_64_hash_string(code_for_hash);

    // Max value of uint64_t is 18446744073709551615, 20 digits,
    // so we make the length of hash_str to 20 so that all the filenames of
    // files generated will be the same length
    // string hash_str = std::to_string(hash);
    // hash_str = string(20 - hash_str.length(), '0') + hash_str;

    string hash_str = u64_to_string_base64x(hash);

    // write temporary c++ code in this file
    const string out_cpp_name = hash_str + ".cpp";
    // compile output file
    const string out_bin_name = hash_str + ".bin";

    // the source code full path
    src_path = cache_dir / SUB_DIR_CACHE / out_cpp_name;

    // the executable full path
    bin_path = cache_dir / SUB_DIR_CACHE / out_bin_name;
}

void Paths::get_src_bin_full_path_permanent(const std::string name,
                                            Path &src_path,
                                            Path &bin_path,
                                            Path &desc_path) const {
    src_path = cache_dir / sub_permanent_dir / (name + ".cpp");
    bin_path = cache_dir / sub_permanent_dir / (name + ".bin");
    desc_path = cache_dir / sub_permanent_dir / (name + ".desc");
}

} // namespace rcc
