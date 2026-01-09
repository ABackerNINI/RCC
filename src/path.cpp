#include "path.h"

#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Path::Path(const std::string &path) : path_(check_whitespaces(path)) {}

Path::Path(const char *path) : path_(check_whitespaces(path)) {}

Path::Path() : path_("") {}

Path &Path::join(const std::string &path) {
    return join(Path(path));
}

Path &Path::join(const char *path) {
    return join(Path(path));
}

Path &Path::join(const Path &other) {
    if (other.path_.empty()) {
        return *this;
    } else if (this->path_.empty()) {
        this->path_ = other.path_;
        return *this;
    }

    this->path_ = check_whitespaces(this->get_plain_path() + "/" + other.get_plain_path());

    return *this;
}

const std::string &Path::string() const {
    return this->path_;
}

// Return the path without quotes and trailing slash.
std::string Path::get_plain_path() const {
    if (this->path_.empty()) {
        return this->path_;
    }

    if (this->path_.front() == '"') {
        std::string plain_path = this->path_.substr(1, this->path_.length() - 2);
        if (plain_path.back() == '/' || plain_path.back() == '\\') {
            plain_path.pop_back(); // Remove trailing slash
        }
        return plain_path;
    } else if (this->path_.back() == '/' || this->path_.back() == '\\') {
        std::string plain_path = this->path_;
        plain_path.pop_back(); // Remove trailing slash
        return plain_path;
    }

    return this->path_;
}

bool Path::exists() const {
    struct stat buffer;
    return (stat(this->path_.c_str(), &buffer) == 0);
}

bool Path::is_dir() const {
    struct stat buffer;
    if (stat(this->path_.c_str(), &buffer) == 0) {
        return S_ISDIR(buffer.st_mode);
    }
    return false;
}

bool Path::is_file() const {
    struct stat buffer;
    if (stat(this->path_.c_str(), &buffer) == 0) {
        return S_ISREG(buffer.st_mode);
    }
    return false;
}

Path Path::parent() const {
    size_t pos = path_.find_last_of('/');
    if (pos == std::string::npos)
        return Path(".");
    if (pos == 0)
        return Path("/");
    return Path(path_.substr(0, pos));
}

std::string Path::filename() const {
    size_t pos = path_.find_last_of('/');
    if (pos == std::string::npos)
        return path_;
    return path_.substr(pos + 1);
}

std::string Path::extension() const {
    std::string name = filename();
    size_t pos = name.find_last_of('.');
    if (pos == std::string::npos)
        return "";
    return name.substr(pos);
}

std::string Path::stem() const {
    std::string name = filename();
    size_t pos = name.find_last_of('.');
    if (pos == std::string::npos)
        return name;
    return name.substr(0, pos);
}

bool Path::is_absolute() const {
    return !path_.empty() && path_[0] == '/';
}

std::string Path::normalize(const std::string &path) {
    if (path.empty()) {
        return ".";
    }

    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;

    while (std::getline(ss, part, '/')) {
        if (part.empty() || part == ".")
            continue;
        if (part == "..") {
            if (!parts.empty())
                parts.pop_back();
        } else {
            parts.push_back(part);
        }
    }

    if (parts.empty()) {
        return path[0] == '/' ? "/" : ".";
    }

    std::string result;
    if (path[0] == '/') {
        result = "/";
    }

    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0)
            result += "/";
        result += parts[i];
    }

    return result;
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
