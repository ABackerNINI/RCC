#ifndef __FMT_H__
#define __FMT_H__

// IWYU pragma: begin_keep

#include "libs/fmt/color.h"
#include "libs/fmt/ostream.h"
#include "libs/fmt/ranges.h"
#include "libs/fmt/std.h"
#include <unistd.h> // isatty

// IWYU pragma: end_keep

// A patch for `fmt` library to make it work with TTYs.
namespace fmt_patch {

// It is a wrapper around `fmt::print` that checks if the file descriptor is a
// TTY, and if not, it will not print the color codes.
template <typename... T> void cprint(FILE *fp, fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
    if (isatty(fileno(fp))) {
        fmt::print(fp, ts, fmt, std::forward<T>(args)...);
    } else {
        fmt::print(fp, fmt, std::forward<T>(args)...);
    }
}

// It is a wrapper around `fmt::print` that checks if the file descriptor is a
// TTY, and if not, it will not print the color codes.
template <typename... T> void cprint(fmt::text_style ts, fmt::format_string<T...> fmt, T &&...args) {
    fmt_patch::cprint(stdout, ts, fmt, std::forward<T>(args)...);
}

//*===================================================================================================================*/

template <typename T> struct patched_styled_arg : fmt::detail::view {
    const T &value;
    fmt::text_style style;
    mutable bool enable;
    patched_styled_arg(const T &v, fmt::text_style s) : value(v), style(s), enable(true) {}
};

template <typename T>
FMT_CONSTEXPR auto cstyled(const T &value, fmt::text_style ts) -> patched_styled_arg<fmt::remove_cvref_t<T>> {
    return patched_styled_arg<fmt::remove_cvref_t<T>>{value, ts};
}

template <typename T> void patch_styled_arg(FILE *fp, const patched_styled_arg<T> &styled_arg) {
    styled_arg.enable = !isatty(fileno(fp));
}

template <typename T> void patch_styled_arg(FILE *fp, T &&styled_arg) {
    (void)fp;
    (void)styled_arg;
}

// It is a wrapper around `fmt::print` that checks if the file descriptor is a
// TTY, and if not, it will not print the color codes.
template <typename... T> void cprint(FILE *fp, fmt::format_string<T...> fmt, T &&...args) {
    // TODO: fold-expression is not available before C++17
    // Call patch_styled_arg for each argument
    (patch_styled_arg(fp, args), ...);

    fmt::print(fp, fmt, std::forward<T>(args)...);
}

// It is a wrapper around `fmt::print` that checks if the file descriptor is a
// TTY, and if not, it will not print the color codes.
template <typename... T> void cprint(fmt::format_string<T...> fmt, T &&...args) {
    fmt_patch::cprint(stdout, fmt, std::forward<T>(args)...);
}

} // namespace fmt_patch

namespace fmt {

// formatter for `fmt_patch::patched_styled_arg` that will print the text with
// style if it is enabled, and the text only if it is not.
template <typename T, typename Char> struct formatter<fmt_patch::patched_styled_arg<T>, Char> : formatter<T, Char> {
    template <typename FormatContext>
    auto format(const fmt_patch::patched_styled_arg<T> &arg, FormatContext &ctx) const -> decltype(ctx.out()) {
        const auto &ts = arg.enable ? arg.style : fmt::text_style();
        auto out = ctx.out();

        bool has_style = false;
        if (ts.has_emphasis()) {
            has_style = true;
            auto emphasis = detail::make_emphasis<Char>(ts.get_emphasis());
            out = detail::copy<Char>(emphasis.begin(), emphasis.end(), out);
        }
        if (ts.has_foreground()) {
            has_style = true;
            auto foreground = detail::make_foreground_color<Char>(ts.get_foreground());
            out = detail::copy<Char>(foreground.begin(), foreground.end(), out);
        }
        if (ts.has_background()) {
            has_style = true;
            auto background = detail::make_background_color<Char>(ts.get_background());
            out = detail::copy<Char>(background.begin(), background.end(), out);
        }
        out = formatter<T, Char>::format(arg.value, ctx);
        if (has_style) {
            auto reset_color = string_view("\x1b[0m");
            out = detail::copy<Char>(reset_color.begin(), reset_color.end(), out);
        }
        return out;
    }
};

} // namespace fmt

#define print fmt_patch::cprint
#define styled fmt_patch::cstyled

using fmt::bg;
using fmt::color;
using fmt::emphasis;
using fmt::fg;
using fmt::format;
using fmt::terminal_color;
using fmt::text_style;

// If the file descriptor is a TTY, return the text style, otherwise return a
// default text style. This is used to make the output look nice in terminals.
// If the output is redirected to a file, the colors are disabled.
// TODO: remove it
inline text_style TTY_TS(text_style ts, FILE *fp) {
    // return isatty(fileno(fp)) ? ts : text_style();
    (void)fp;
    return ts;
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
