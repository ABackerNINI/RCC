#ifndef __RCC_PATH_H__
#define __RCC_PATH_H__

#include <string>

#include "libs/ghc/fs_std_fwd.hpp"

// Represent a path in the filesystem. Maybe use std::filesystem in C++17?
//* Currently, this supports only Linux.
class Path {
  public:
    Path();
    Path(const char *path);
    Path(const std::string &path);

    // Join a path with another path.
    Path &join(const std::string &other);

    // Join a path with another path.
    Path &join(const char *other);

    // Join a path with another path.
    Path &join(const Path &other);

    Path operator/(const std::string &other) const;
    Path operator+(const std::string &other) const;
    Path operator/(const Path &other) const;
    Path operator+(const Path &other) const;

    Path parent() const;
    std::string filename() const;
    std::string extension() const;
    std::string stem() const;
    bool is_absolute() const;

    // Get the path as a string.
    const std::string &string() const;

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

    static std::string normalize(const std::string &path);

  private:
    // Return the path with quotes around it if it contains spaces.
    static std::string check_whitespaces(const std::string &path);

  private:
    std::string path_;
};

std::string operator+(const Path &lhs, const std::string &rhs);
std::string operator+(const std::string &lhs, const Path &rhs);

#endif // __RCC_PATH_H__
