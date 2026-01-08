#ifndef __RCC_PCH_H__
#define __RCC_PCH_H__

// IWYU pragma: begin_keep

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <ctime>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "libs/CLI11.hpp"
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

// namespace rcc {
//
// class fmt_printer {
//   public:
//     template <typename... T> fmt_printer &c(fmt::format_string<T...> fmt, T &&...args) {
//         fmt::print(fmt, std::forward<T>(args)...);
//         return *this;
//     }
//
//     template <typename... T> fmt_printer &c(fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
//         fmt::print(ts, fmt, std::forward<T>(args)...);
//         return *this;
//     }
// };
//
// class fmt_printer_ostream {
//   public:
//     fmt_printer_ostream(std::ostream &os) : os(os) {}
//
//     template <typename... T> fmt_printer_ostream &c(fmt::format_string<T...> fmt, T &&...args) {
//         fmt::print(os, fmt, std::forward<T>(args)...);
//         return *this;
//     }
//
//     template <typename... T> fmt_printer_ostream &c(fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
//         fmt::print(os, ts, fmt, std::forward<T>(args)...);
//         return *this;
//     }
//
//   private:
//     std::ostream &os;
// };
//
// class fmt_printer_file {
//   public:
//     fmt_printer_file(FILE *f) : f(f) {}
//
//     template <typename... T> fmt_printer_file &c(fmt::format_string<T...> fmt, T &&...args) {
//         fmt::print(f, fmt, std::forward<T>(args)...);
//         return *this;
//     }
//
//     template <typename... T> fmt_printer_file &c(fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
//         fmt::print(f, ts, fmt, std::forward<T>(args)...);
//         return *this;
//     }
//
//   private:
//     FILE *f;
// };
//
// template <typename... T> fmt_printer print(fmt::format_string<T...> fmt, T &&...args) {
//     fmt::print(fmt, std::forward<T>(args)...);
//     return fmt_printer();
// }
//
// template <typename... T> fmt_printer print(fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
//     fmt::print(ts, fmt, std::forward<T>(args)...);
//     return fmt_printer();
// }
//
// template <typename... T> fmt_printer_ostream print(std::ostream &os, fmt::format_string<T...> fmt, T &&...args) {
//     fmt::print(os, fmt, std::forward<T>(args)...);
//     return fmt_printer_ostream(os);
// }
//
// template <typename... T>
// fmt_printer_ostream print(std::ostream &os, fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
//     fmt::print(os, ts, fmt, std::forward<T>(args)...);
//     return fmt_printer_ostream(os);
// }
//
// template <typename... T> fmt_printer_file print(FILE *&f, fmt::format_string<T...> fmt, T &&...args) {
//     fmt::print(f, fmt, std::forward<T>(args)...);
//     return fmt_printer_file(f);
// }
//
// template <typename... T>
// fmt_printer_file print(FILE *&f, fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
//     fmt::print(f, ts, fmt, std::forward<T>(args)...);
//     return fmt_printer_file(f);
// }
//
// } // namespace rcc

// #define print fmt::print

using fmt::print;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

#endif // __RCC_PCH_H__
