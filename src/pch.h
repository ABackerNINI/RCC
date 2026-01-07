#ifndef __RCC_PCH_H__
#define __RCC_PCH_H__

// IWYU pragma: begin_keep

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <ctime>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "libs/CLI11.hpp"
#include "libs/debug.h"
#include "libs/fmt/args.h"
#include "libs/fmt/base.h"
#include "libs/fmt/chrono.h"
#include "libs/fmt/color.h"
#include "libs/fmt/compile.h"
#include "libs/fmt/core.h"
#include "libs/fmt/format.h"
#include "libs/fmt/os.h"
#include "libs/fmt/ostream.h"
#include "libs/fmt/printf.h"
#include "libs/fmt/ranges.h"
#include "libs/fmt/std.h"
#include "libs/fmt/xchar.h"
#include "libs/rang.hpp"

// IWYU pragma: end_keep

class fmt_printer {
  public:
    template <typename... T> fmt_printer &print(T &&...args) {
        fmt::print(std::forward<T>(args)...);
        return *this;
    }
};

class fmt_printer2 {
  public:
    fmt_printer2(std::ostream &os) : os(os) {}

    template <typename... T> fmt_printer2 &print(T &&...args) {
        fmt::print(os, std::forward<T>(args)...);
        return *this;
    }

  private:
    std::ostream &os;
};

template <typename... T> fmt_printer print(T &&...args) {
    fmt::print(std::forward<T>(args)...);
    return fmt_printer();
}

template <typename... T> fmt_printer2 print(std::ostream &os, T &&...args) {
    fmt::print(os, std::forward<T>(args)...);
    return fmt_printer2(os);
}

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

#endif // __RCC_PCH_H__
