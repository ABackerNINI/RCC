#ifndef __RCC_TEMPLATE_H__
#define __RCC_TEMPLATE_H__

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

#define FOR(l, r)  for (int i = l; i < r; ++i)
#define FORR(r, l) for (int i = r; i >= l; --i)

#define FORI(l, r) for (int i = l; i < r; ++i)
#define FORJ(l, r) for (int j = l; j < r; ++j)
#define FORK(l, r) for (int k = l; k < r; ++k)

#define FORRI(r, l) for (int i = r; i >= l; --i)
#define FORRJ(r, l) for (int j = r; j >= l; --j)
#define FORRK(r, l) for (int k = r; k >= l; --k)

/*==========================================================================*/

// Wrapper macro that returns the first character passed in.
#define CH(c) __ch(#c)

// Return the first character passed in.
inline char __ch(const char *c) { return c[0]; }

// Wrapper function of system(const char *);
inline int system(const std::string &cmd) { return system(cmd.c_str()); }

/*==========================================================================*/

// Split string into tokens like strtok().
inline std::vector<std::string>
split(const std::string &s, const std::string &delims = " \t\r\n\v\f") {
    std::vector<std::string> words;

    bool has_content = false;
    size_t l = 0;
    for (size_t i = 0; i < s.length(); ++i) {
        if (delims.find(s[i]) != std::string::npos) {
            if (has_content) {
                words.push_back(s.substr(l, i - l));
                has_content = false;
            }
        } else {
            if (!has_content) {
                l = i;
                has_content = true;
            }
        }
    }
    if (has_content) {
        words.push_back(s.substr(l));
    }

    return words;
}

// Split string into tokens, support quoted string.
inline std::vector<std::string>
split_quoted(const std::string &s,
             const std::string &delims = " \t\r\n\v\f",
             const std::string &quotes = "\"'") {
    std::vector<std::string> words; // The list to be created
    // // Use a string stream to read individual words
    // std::istringstream is(line);
    // std::string word;
    // while (is >> std::quoted(word)) {
    //     words.push_back(word);
    // }
    // return words;

    // TODO:

    bool has_content = false;
    size_t l = 0;
    for (size_t i = 0; i < s.length(); ++i) {
        if (delims.find(s[i]) != std::string::npos) {
            if (has_content) {
                words.push_back(s.substr(l, i - l));
                has_content = false;
            }
        } else if (quotes.find(s[i]) != std::string::npos) {

        } else {
            if (!has_content) {
                l = i;
                has_content = true;
            }
        }
    }
    if (has_content) {
        words.push_back(s.substr(l));
    }

    return words;
}

/*==========================================================================*/

// Show the ascii value of the given characters.
inline void ascii(const char *characters = NULL) {
    if (characters) {
        const char *p = characters;
        while (*p) {
            std::cout << *p << ": " << (int)*p << std::endl;
            ++p;
        }
        std::cout << std::endl;
    } else {
        // TODO:
    }
}

/*==========================================================================*/

#define TRAVERSAL_CALLBACK_RETURN_CODE_CONTINUE 0
#define TRAVERSAL_CALLBACK_RETURN_CODE_SKIP_DIR 1
#define TRAVERSAL_CALLBACK_RETURN_CODE_BREAK    -1

/**
 * @brief Traverse files and directories in directory 'dir'.
 *
 * @example
 *   TRAVERSAL("../.", {
 *       cout << path << ", " << filename << (is_file ? "" : " [dir]") << endl;
 *       return false;
 *   });
 */
#define TRAVERSAL(dir, ...)                                                    \
    traversal(dir,                                                             \
              [&](const std::string &path,                                     \
                  const std::string &filename,                                 \
                  bool is_file) -> int { __VA_ARGS__ });

/**
 * @brief Helper function for traverse files in directory 'dir' recursively.
 * This will call the callback function with every file or directory.
 *
 * @return
 *      @retval 0 if no errors.
 *      @retval -1 if error.
 */
template <typename traversal_callback_t>
int traversal_helper(std::string &dir, traversal_callback_t callback) {
    DIR *p_dir = NULL;
    struct dirent *p_entry = NULL;
    struct stat statbuf;

    if ((p_dir = opendir(dir.c_str())) == NULL) {
        std::cerr << "opendir: " << strerror(errno) << "'" << dir << "'"
                  << std::endl;
        return -1;
    }

    dir.push_back('/');

    while (NULL != (p_entry = readdir(p_dir))) {
        size_t old_len = dir.length();
        dir += p_entry->d_name;
        if (lstat(dir.c_str(), &statbuf) == 0) {
            if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { /* dir */
                /* ignore "." and ".." */
                if (strcmp(".", p_entry->d_name) != 0 &&
                    strcmp("..", p_entry->d_name) != 0) {

                    int ret;
                    if ((ret = callback(dir, p_entry->d_name, false)) ==
                        TRAVERSAL_CALLBACK_RETURN_CODE_BREAK) {
                        break;
                    }

                    if (ret != TRAVERSAL_CALLBACK_RETURN_CODE_SKIP_DIR) {
                        traversal_helper(dir, callback);
                    }
                }
            } else { /* file */
                if (callback(dir, p_entry->d_name, true)) {
                    break;
                }
            }
        }
        dir.resize(old_len);
    }

    closedir(p_dir);

    return 0;
}

/**
 * @brief Traverse file tree in directory 'dir' recursively.
 */
template <typename traversal_callback_t>
void traversal(const std::string &dir, traversal_callback_t callback) {
    if (dir.length() >= FILENAME_MAX) {
        fprintf(stderr, "Error: filename too long!\n");
        return;
    }

    std::string path = dir;

    // Incase called with '/' at the end
    if (path.back() == '/') {
        path.pop_back();
    }

    traversal_helper(path, callback);
}

// TODO: regex support

#endif // __RCC_TEMPLATE_H__
