#include "utils.h"
#include <algorithm>
#include <cassert>

namespace rcc {

bool starts_with(const std::string &str, const std::string &prefix) {
    return str.length() >= prefix.length() && str.compare(0, prefix.length(), prefix) == 0;
}

bool ends_with(const std::string &str, const std::string &suffix) {
    return str.length() >= suffix.length() && str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

uint64_t fnv1a_64_hash_string(const std::string &str) {
    const uint64_t FNV_PRIME = 1099511628211ULL; // 2^40 + 2^8 + 0xb3
    const uint64_t FNV_OFFSET = 14695981039346656037ULL; // 2^64 - 2^32 - 2^16 - 2^8 - 1

    uint64_t hash = FNV_OFFSET;

    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= FNV_PRIME;
    }

    return hash;
}

std::string u64_to_string_base64x(uint64_t val) {
    static constexpr char base64x_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz"
                                            "0123456789+_";

    std::string result(11, '0');
    for (int i = 0; i < 11; i++) {
        result[i] = base64x_chars[val & 0x3F];
        val >>= 6;
    }

    return result;
}

std::string vector_to_string(const std::vector<std::string> &vec,
                             const std::string &sep,
                             const std::string &default_for_empty,
                             bool keep_trailing_sep) {
    if (vec.empty()) {
        return default_for_empty + (keep_trailing_sep ? sep : "");
    }

    std::string result;
    result.reserve(vec.size() * (vec[0].length() + sep.length())); // Pre-allocate memory

    for (const auto &item : vec) {
        result += item + sep;
    }

    // Remove trailing separator if needed
    if (!keep_trailing_sep && result.size() > 0) {
        for (size_t i = 0; i < sep.size(); i++) {
            result.pop_back();
        }
    }

    return result;
}

std::string &replace_all(std::string &str, const std::string &find, const std::string &replacement) {
    size_t pos = 0;
    while ((pos = str.find(find, pos)) != std::string::npos) {
        str.replace(pos, find.length(), replacement);
        pos += replacement.length(); // Move past the replacement
    }
    return str;
}

std::string escapeshellarg(const std::string &arg) {
    // If the argument contains single quotes, escape them
    // * Why use `'\\''` instead of `\\'`? Because the shell will interpret anything
    // * inside the single quotes as literal, so `\\'` will be interpreted
    // * as `\\` and the end single quote, which is not what we want.
    // ! THINGS MIGHT BE DIFFERENT ON WINDOWS
    std::string escaped = arg;
    replace_all(escaped, "'", "'\\''");

    // Wrap the argument inside single quotes, this keeps it be interpreted as is
    // * DO NOT use double quotes
    return "'" + escaped + "'";
}

std::string escapeforprint(const std::string &input) {
    std::ostringstream oss;
    for (unsigned char ch : input) {
        switch (ch) {
        case '\n':
            oss << "\\n";
            break;
        case '\r':
            oss << "\\r";
            break;
        case '\t':
            oss << "\\t";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\v':
            oss << "\\v";
            break;
        case '\a':
            oss << "\\a";
            break;
        case '\b':
            oss << "\\b";
            break;
        // case '\\':
        //     oss << "\\\\";
        //     break;
        // case '\"':
        //     oss << "\\\"";
        //     break;
        // case '\'':
        //     oss << "\\\'";
        //     break;
        default:
            if (ch >= 32 && ch < 127) { // printable ASCII characters
                oss << ch;
            } else { // non-printable characters
                oss << "{\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ch) << "}";
            }
            break;
        }
    }
    return oss.str();
}

std::vector<fs::path> find_files(const fs::path &dir, const std::vector<std::string> &extensions) {
    std::vector<fs::path> files;
    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            if (std::find(extensions.begin(), extensions.end(), entry.path().extension().string()) !=
                extensions.end()) {
                files.push_back(entry.path());
            }
        }
    }
    return files; // Return the vector of files
}

size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2) {
#define DP(i, j) dp[(i) * (len2 + 1) + (j)]
    // dp should be longer than dp[(len1+1)*(len2+1)]
    size_t *dp = new size_t[(len1 + 1) * (len2 + 1)];
    for (size_t i = 0; i <= len1; ++i) {
        DP(i, 0) = i;
    }
    for (size_t i = 0; i <= len2; ++i) {
        DP(0, i) = i;
    }

    size_t flag;
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            flag = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            DP(i, j) = std::min(DP(i - 1, j) + 1, std::min(DP(i, j - 1) + 1, DP(i - 1, j - 1) + flag));
        }
    }

    size_t ret = DP(len1, len2);

    delete[] dp;

    return ret;
#undef DP
}

size_t edit_distance(const std::string &s1, const std::string &s2) {
    return edit_distance(s1.c_str(), s1.length(), s2.c_str(), s2.length());
}

fmt::text_style duration_color(double green_time_ms, double red_time_ms, double duration_in_ms) {
    assert(green_time_ms < red_time_ms);

    uint8_t r, g, b = 0;
    if (duration_in_ms <= green_time_ms) {
        r = 0;
        g = 255;
    } else if (duration_in_ms >= red_time_ms) {
        r = 255;
        g = 0;
    } else {
        r = static_cast<uint8_t>(duration_in_ms * 255 / (red_time_ms - green_time_ms));
        g = 255 - r;
    }
    return fg(fmt::rgb(r, g, b)) | emphasis::bold;
}

} // namespace rcc
