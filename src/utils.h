#ifndef __RCC_UTILS_H__
#define __RCC_UTILS_H__

#include "libs/ghc/fs_std.hpp" // IWYU pragma: keep
#include <cstdint>
#include <string>
#include <vector>

namespace rcc {

// Hash a string using the FNV-1a algorithm.
uint64_t fnv1a_64_hash_string(const std::string &str);

// Convert an uint64_t to a string in a variant of base 64.
// The resulting string will be 11 characters long.
// Characters used are: A-Z, a-z, 0-9, +, _.
std::string u64_to_string_base64x(uint64_t val);

// Concatenate a vector of strings into one string with a separator.
// If the vector is empty, return the default_for_empty string.
std::string vector_to_string(const std::vector<std::string> &vec,
                             const std::string &sep = " ",
                             const std::string &default_for_empty = "");

// Find all files with the given extensions in the given directory and all subdirectories.
std::vector<fs::path> find_files(const fs::path &dir, const std::vector<std::string> &extensions);

// Calculate the edit distance between two strings, C style.
size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2);

// Calculate the edit distance between two strings, C++ style.
size_t edit_distance(const std::string &s1, const std::string &s2);

} // namespace rcc

#endif // __RCC_UTILS_H__
