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
// If fp is not specified, stdout is used.
inline text_style TTY_TS(text_style ts, FILE *fp = stdout) {
    if (isatty(fileno(fp))) {
        return ts;
    } else {
        return text_style();
    }
}

constexpr text_style red_bold = fg(terminal_color::red) | emphasis::bold;
constexpr text_style green_bold = fg(terminal_color::green) | emphasis::bold;
constexpr text_style yellow_bold = fg(terminal_color::yellow) | emphasis::bold;
constexpr text_style blue_bold = fg(terminal_color::blue) | emphasis::bold;
constexpr text_style magenta_bold = fg(terminal_color::magenta) | emphasis::bold;
constexpr text_style cyan_bold = fg(terminal_color::cyan) | emphasis::bold;
constexpr text_style white_bold = fg(terminal_color::white) | emphasis::bold;

#endif // __FMT_H__
