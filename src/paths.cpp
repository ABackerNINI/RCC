#include "pch.h"

#include "paths.h"
#include "utils.h"

namespace rcc {

Path::Path(const std::string &path) : path(check_whitespaces(path)) {}

Path::Path(const char *path) : path(check_whitespaces(path)) {}

Path::Path() : path("") {}

Path &Path::join(const std::string &path) {
    return join(Path(path));
}

Path &Path::join(const char *path) {
    return join(Path(path));
}

Path &Path::join(const Path &other) {
    if (other.path.empty()) {
        return *this;
    } else if (this->path.empty()) {
        this->path = other.path;
        return *this;
    }

    this->path = check_whitespaces(this->get_plain_path() + "/" + other.get_plain_path());

    return *this;
}

const std::string &Path::get_path() const {
    return this->path;
}

// Return the path without quotes and trailing slash.
std::string Path::get_plain_path() const {
    if (this->path.empty()) {
        return this->path;
    }

    if (this->path.front() == '"') {
        std::string plain_path = this->path.substr(1, this->path.length() - 2);
        if (plain_path.back() == '/' || plain_path.back() == '\\') {
            plain_path.pop_back(); // Remove trailing slash
        }
        return plain_path;
    } else if (this->path.back() == '/' || this->path.back() == '\\') {
        std::string plain_path = this->path;
        plain_path.pop_back(); // Remove trailing slash
        return plain_path;
    }

    return this->path;
}

bool Path::exists() const {
    struct stat buffer;
    return (stat(this->path.c_str(), &buffer) == 0);
}

bool Path::is_dir() const {
    struct stat buffer;
    if (stat(this->path.c_str(), &buffer) == 0) {
        return S_ISDIR(buffer.st_mode);
    }
    return false;
}

bool Path::is_file() const {
    struct stat buffer;
    if (stat(this->path.c_str(), &buffer) == 0) {
        return S_ISREG(buffer.st_mode);
    }
    return false;
}

std::string Path::check_whitespaces(const std::string &path) {
    for (char c : path) {
        if (isspace(c)) {
            return std::string("\"") + path + "\"";
        }
    }
    return path; // No whitespaces found
}

// Read file line by line and put them together in a string.
string Path::read_file() const {
    std::ifstream infile(path);
    if (!infile) {
        cerr << "Can't open file: " << path << endl;
        exit(1);
    }

    string content;
    content.reserve(1024);

    char buf[1024];
    std::streamsize bytes_read = 0;
    while (infile.read(buf, 1024), (bytes_read = infile.gcount()) > 0) {
        content.append(buf, bytes_read);
    }

    if (!infile.eof()) {
        if (infile.fail()) {
            throw std::runtime_error("Error: Failed to read from file.");
        } else if (infile.bad()) {
            throw std::runtime_error("Error: Bad stream state.");
        }
    }

    return content;
}

// Write string to a file.
void Path::write_file(const string &content) const {
    std::ofstream outfile(path);
    if (!outfile) {
        cerr << "Can't open file: " << path << endl;
        exit(1);
    }
    outfile << content;
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

    this->template_path = sub_templates_dir;
    this->template_path.join("rcc_template.cpp");
    this->template_header_path = sub_templates_dir;
    this->template_header_path.join("rcc_template.hpp");
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

    // Check if the cache directory exists, if not, exit
    if (access(cache_dir.c_str(), F_OK) != 0) {
        cerr << "Cache directory does not exist: " << cache_dir << endl;
        cerr << "Please reinstall rcc." << endl;
        exit(1);
    }

    // Check if the templates directory exists, if not, exit
    if (access(sub_templates_dir.c_str(), F_OK) != 0) {
        cerr << "Templates directory does not exist: " << cache_dir + "/" + SUB_DIR_TEMPLATES << endl;
        cerr << "Please reinstall rcc." << endl;
        exit(1);
    }

    // Check if the sub cache directory exists, if not, create it
    if (access(sub_cache_dir.c_str(), F_OK) != 0) {
        if (mkdir(sub_cache_dir.c_str(), 0755) != 0) {
            cerr << "Failed to create cache directory: " << sub_cache_dir << endl;
            exit(1);
        }
    }

    this->cache_dir = cache_dir;
    this->sub_cache_dir = sub_cache_dir;
    this->sub_templates_dir = sub_templates_dir;
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
    src_path = cache_dir;
    src_path.join(SUB_DIR_CACHE).join(out_cpp_name);

    // the executable full path
    bin_path = cache_dir;
    bin_path.join(SUB_DIR_CACHE).join(out_bin_name);
}
} // namespace rcc
