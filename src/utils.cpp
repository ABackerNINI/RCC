#include "utils.h"

namespace rcc {

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
    std::string result;
    result.reserve(11);

    for (int i = 0; i < 11; i++) {
        result += base64x_chars[val & 0x3F];
        val >>= 6;
    }

    return result;
}

std::string vector_to_string(const std::vector<std::string> &vec,
                             const std::string &sep,
                             const std::string &default_for_empty) {
    if (vec.empty()) {
        return default_for_empty;
    }

    std::string result = "";
    for (const auto &item : vec) {
        result += item + sep;
    }
    if (result.size() > 0) {
        for (size_t i = 0; i < sep.size(); i++) {
            result.pop_back();
        }
    }
    return result;
}

} // namespace rcc
