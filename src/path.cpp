// This header should be included before the `libs/ghc/fs_std_fwd.hpp`
#include "libs/ghc/fs_std_impl.hpp"

#include "path.h"

#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>

std::string Path::check_whitespaces(const std::string &path) {
    for (char c : path) {
        if (isspace(c)) {
            return std::string("\"") + path + "\"";
        }
    }
    return path; // No whitespaces found
}

// Read file line by line and put them together in a string.
std::string Path::read_file() const {
    std::ifstream infile(path_);
    if (!infile) {
        std::cerr << "Can't open file: " << path_ << std::endl;
        exit(1);
    }

    std::string content;
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
void Path::write_file(const std::string &content) const {
    std::ofstream outfile(path_);
    if (!outfile) {
        std::cerr << "Can't open file: " << path_ << std::endl;
        exit(1);
    }
    outfile << content;
}
