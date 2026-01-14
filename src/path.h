#ifndef __RCC_PATH_H__
#define __RCC_PATH_H__

#include <string>

#include "libs/ghc/fs_std_fwd.hpp" // IWYU pragma: keep

// Wrapper class for fs::path with some additional utility functions
class Path {
  public:
    Path() : path_() {}
    Path(const char *path) : path_(path) {}
    Path(const std::string &path) : path_(path) {}
    Path(const fs::path &path) : path_(path) {}

    Path operator/(const Path &other) const { return Path(path_ / other.path_); }
    Path &operator/=(const Path &other) {
        path_ /= other.path_;
        return *this;
    }

    void clear() { path_.clear(); }

    Path &make_preferred() {
        path_.make_preferred();
        return *this;
    }

    Path &remove_filename() {
        path_.remove_filename();
        return *this;
    }

    Path &replace_filename(const Path &replacement) {
        path_.replace_filename(replacement.path_);
        return *this;
    }

    Path &replace_extension() {
        path_.replace_extension();
        return *this;
    }

    void swap(Path &other) { path_.swap(other.path_); }

    const char *c_str() const { return path_.c_str(); }
    std::string native() const { return path_.native(); }
    operator std::string() const { return path_; }
    std::string string() const { return path_.string(); }
    std::string generic_string() const { return path_.generic_string(); }

    int compare(const Path &other) const { return path_.compare(other.path_); }

    Path lexically_normal() { return path_.lexically_normal(); }
    Path lexically_relative(const Path &base) const { return path_.lexically_relative(base.path_); }
    Path lexically_proximate(const Path &base) const { return path_.lexically_proximate(base.path_); }

    Path root_name() const { return path_.root_name(); }
    Path root_directory() const { return path_.root_directory(); }
    Path root_path() const { return path_.root_path(); }
    Path relative_path() const { return path_.relative_path(); }
    Path parent_path() const { return path_.parent_path(); }
    std::string filename() const { return path_.filename().string(); }
    std::string stem() const { return path_.stem().string(); }
    std::string extension() const { return path_.extension().string(); }

    bool empty() const { return path_.empty(); }

    bool has_root_path() const { return path_.has_root_path(); }
    bool has_root_name() const { return path_.has_root_name(); }
    bool has_root_directory() const { return path_.has_root_directory(); }
    bool has_relative_path() const { return path_.has_relative_path(); }
    bool has_parent_path() const { return path_.has_parent_path(); }
    bool has_filename() const { return path_.has_filename(); }
    bool has_stem() const { return path_.has_stem(); }
    bool has_extension() const { return path_.has_extension(); }

    bool is_absolute() const { return path_.is_absolute(); }
    bool is_relative() const { return path_.is_relative(); }

    /*==========================================================================*/
    /* ADDITIONAL METHODS */
    /*==========================================================================*/

    const fs::path &get_path() const { return path_; } 

    bool exists() const { return fs::exists(path_); }
    bool is_dir() const { return fs::is_directory(path_); }
    bool is_file() const { return fs::is_regular_file(path_); }
    std::size_t file_size() const { return fs::file_size(path_); }

    void remove() const { fs::remove(path_); }
    void remove_all() const { fs::remove_all(path_); }
    void copy(const Path &new_path) const { fs::copy(path_, new_path.path_); }
    void rename(const Path &new_path) {
        fs::rename(path_, new_path.path_);
        path_ = new_path.path_;
    }

    // Return the path with quotes around it if it contains spaces.
    std::string quote_if_needed() const { return check_whitespaces(path_.string()); }

    // Read the file at the path. Return the contents as a string.
    std::string read_file() const;

    // Write the given content to the file at the path.
    void write_file(const std::string &content) const;

  private:
    // Return the path with quotes around it if it contains spaces.
    static std::string check_whitespaces(const std::string &path);

  private:
    fs::path path_;
};

#endif // __RCC_PATH_H__
