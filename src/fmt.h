#ifndef __FMT_H__
#define __FMT_H__

// IWYU pragma: begin_keep

#include "libs/fmt/color.h"
#include "libs/fmt/ostream.h"
#include "libs/fmt/ranges.h"
#include "libs/fmt/std.h"
#include <unistd.h> // isatty

// IWYU pragma: end_keep

using namespace fmt;

// If the file descriptor is a TTY, return the text style, otherwise return a
// default text style. This is used to make the output look nice in terminals.
// If the output is redirected to a file, the colors are disabled.
inline text_style TTY_TS(text_style ts, FILE *fp) {
    return isatty(fileno(fp)) ? ts : text_style();
}

#if __cplusplus >= 201402L
// C++14 or later
constexpr text_style red_bold = fg(terminal_color::red) | emphasis::bold;
constexpr text_style green_bold = fg(terminal_color::green) | emphasis::bold;
#else
// C++11 or earlier
static text_style red_bold = fg(terminal_color::red) | emphasis::bold;
static text_style green_bold = fg(terminal_color::green) | emphasis::bold;
#endif

#endif // __FMT_H__
