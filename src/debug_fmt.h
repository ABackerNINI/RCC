#ifndef __DEBUG_PRINT_FMT_H__
#define __DEBUG_PRINT_FMT_H__

// #define DEBUG

#include "fmt.h" // IWYU pragma: keep

#define __DPF_DO_IT(...)                                                                                               \
    do {                                                                                                               \
        __VA_ARGS__;                                                                                                   \
    } while (0)

#define __DPF_DO_NOTHING(...)                                                                                          \
    do {                                                                                                               \
        if (0) {                                                                                                       \
            __VA_ARGS__;                                                                                               \
        }                                                                                                              \
    } while (0)

#ifdef DEBUG
    #include <cassert>
#endif

#ifdef DEBUG
    #define __DPF_DO_IF_DBG(...) __DPF_DO_IT(__VA_ARGS__)
#else
    #define __DPF_DO_IF_DBG(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#define gprint(...)                                                                                                    \
    __DPF_DO_IF_DBG(fmt::print(stderr, fmt::fg(fmt::color::green), "[DEBUG] "); fmt::print(stderr, __VA_ARGS__);)

#define gprintc(...)                                                                                                   \
    __DPF_DO_IF_DBG(fmt::print(stderr, fmt::fg(fmt::color::green), " > "); fmt::print(stderr, __VA_ARGS__);)

#define gprintx(...) __DPF_DO_IF_DBG(fmt::print(stderr, __VA_ARGS__);)

#define gstmt(...) __DPF_DO_IF_DBG(__VA_ARGS__;)

#define grequire(...) __DPF_DO_IF_DBG(assert(__VA_ARGS__))
#define gassert(...) __DPF_DO_IF_DBG(assert(__VA_ARGS__))
#define gensure(...) __DPF_DO_IF_DBG(assert(__VA_ARGS__))

/*==============================================================================================================*/

#ifndef ENABLE_RUNTIME_DEBUG_LEVEL
    /* Debug level during runtime.
     *
     * You should declare global variable `DBG_LEVEL debug_level;` in your .cpp source
     * file and set `debug_level` to `DBG_LVL::XXX`, macros `gpxxx`,
     * gp`xxx` and gstmt_`xxx` will be executed only if debug_level is not
     * less than the corresponding value.
     *
     * E.g. If you set debug_level to `DBG_LVL::INFO`, gperror/warning/info will be
     * executed but gpdebug/msgdump/excessive will not.
     */
    #define ENABLE_RUNTIME_DEBUG_LEVEL 1
#endif

#if (ENABLE_RUNTIME_DEBUG_LEVEL)
// Note: the name `DEBUG` is already used by the macro, so we use `DEBUG_` instead.
enum class DBG_LEVEL { ERROR = 0, WARNING = 1, INFO = 2, DEBUG_ = 3, MSGDUMP = 4, EXCESSIVE = 5 };

/* Runtime debug level, you should declare it yourself. */
extern DBG_LEVEL debug_level;

    #define DPF_ERROR_COLOR fmt::fg(fmt::terminal_color::red) | fmt::emphasis::bold
    #define DPF_WARNING_COLOR fmt::fg(fmt::terminal_color::yellow) | fmt::emphasis::bold
    #define DPF_INFO_COLOR fmt::fg(fmt::terminal_color::blue) | fmt::emphasis::bold
    #define DPF_DEBUG_COLOR fmt::fg(fmt::terminal_color::green) | fmt::emphasis::bold
    #define DPF_MSGDUMP_COLOR fmt::fg(fmt::terminal_color::white) | fmt::emphasis::bold
    #define DPF_EXCESSIVE_COLOR fmt::fg(fmt::terminal_color::white) | fmt::emphasis::bold

    #include <unistd.h>

    #ifndef RUNTIME_DEBUG_LEVEL
        /* Debug level during compiling, level greater than this will not be compiled into code.*/
        #define RUNTIME_DEBUG_LEVEL 5
    #endif
#endif

#if (ENABLE_RUNTIME_DEBUG_LEVEL)
    #define __DPF_DO_IF_RT_DBG(lvl, ...)                                                                               \
        do {                                                                                                           \
            if (lvl <= debug_level) {                                                                                  \
                __VA_ARGS__;                                                                                           \
            }                                                                                                          \
        } while (0)
#else // ENABLE_RUNTIME_DEBUG_LEVEL
    #define __DPF_DO_IF_RT_DBG(...)                                                                                    \
        do {                                                                                                           \
            if (0) {                                                                                                   \
                __VA_ARGS__;                                                                                           \
            }                                                                                                          \
        } while (0)
#endif // ENABLE_RUNTIME_DEBUG_LEVEL

#define __DPF_PRINT_FUNC(lvl, ts, lvl_str, ...) __DPF_DO_IF_RT_DBG(lvl, __dpf_print_func(ts, lvl_str, __VA_ARGS__))

template <size_t N, typename... T>
void __dpf_print_func(text_style lvl_str_ts, const char (&lvl_str)[N], fmt::format_string<T...> fmt, T &&...args) {
    print(stderr, "{}", styled(lvl_str, lvl_str_ts));
    print(stderr, fmt, std::forward<T>(args)...);
}

template <size_t N, typename... T>
void __dpf_print_func(text_style lvl_str_ts,
                      const char (&lvl_str)[N],
                      text_style ts,
                      fmt::format_string<T...> fmt,
                      T &&...args) {
    print(stderr, "{}", styled(lvl_str, lvl_str_ts));
    print(stderr, ts, fmt, std::forward<T>(args)...);
}

#if (RUNTIME_DEBUG_LEVEL >= 0)
    /* Print error msg. */
    #define gperror(...) __DPF_PRINT_FUNC(DBG_LEVEL::ERROR, DPF_ERROR_COLOR, "[ERROR] ", __VA_ARGS__)
    /* Print continued error msg. */
    #define gperror_c(...) __DPF_PRINT_FUNC(DBG_LEVEL::ERROR, DPF_ERROR_COLOR, " . ", __VA_ARGS__)
    /* Print extended error msg. */
    #define gperror_ex(...) __DPF_PRINT_FUNC(DBG_LEVEL::ERROR, DPF_ERROR_COLOR, "", __VA_ARGS__)
    /* Statements for error. */
    #define gstmt_error(...) __DPF_DO_IF_RT_DBG(DBG_LEVEL::ERROR, __VA_ARGS__)
#else
    /* Print error msg. */
    #define gperror(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print continued error msg. */
    #define gperror_c(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print extended error msg. */
    #define gperror_ex(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Statements for error. */
    #define gstmt_error(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#if (RUNTIME_DEBUG_LEVEL >= 1)
    /* Print warning msg. */
    #define gpwarning(...) __DPF_PRINT_FUNC(DBG_LEVEL::WARNING, DPF_WARNING_COLOR, "[WARNING] ", __VA_ARGS__)
    /* Print continued warning msg. */
    #define gpwarning_c(...) __DPF_PRINT_FUNC(DBG_LEVEL::WARNING, DPF_WARNING_COLOR, " . ", __VA_ARGS__)
    /* Print extended warning msg. */
    #define gpwarning_ex(...) __DPF_PRINT_FUNC(DBG_LEVEL::WARNING, DPF_WARNING_COLOR, "", __VA_ARGS__)
    /* Statements for warning. */
    #define gstmt_warning(...) __DPF_DO_IF_RT_DBG(DBG_LEVEL::WARNING, __VA_ARGS__)
#else
    /* Print warning msg. */
    #define gpwarning(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print continued warning msg. */
    #define gpwarning_c(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print extended warning msg. */
    #define gpwarning_ex(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Statements for warning. */
    #define gstmt_warning(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#if (RUNTIME_DEBUG_LEVEL >= 2)
    /* Print info msg. */
    #define gpinfo(...) __DPF_PRINT_FUNC(DBG_LEVEL::INFO, DPF_INFO_COLOR, "[INFO] ", __VA_ARGS__)
    /* Print continued info msg. */
    #define gpinfo_c(...) __DPF_PRINT_FUNC(DBG_LEVEL::INFO, DPF_INFO_COLOR, " . ", __VA_ARGS__)
    /* Print extended info msg. */
    #define gpinfo_ex(...) __DPF_PRINT_FUNC(DBG_LEVEL::INFO, DPF_INFO_COLOR, "", __VA_ARGS__)
    /* Statements for info. */
    #define gstmt_info(...) __DPF_DO_IF_RT_DBG(DBG_LEVEL::INFO, __VA_ARGS__)
#else
    /* Print info msg. */
    #define gpinfo(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print continued info msg. */
    #define gpinfo_c(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print extended info msg. */
    #define gpinfo_ex(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Statements for info. */
    #define gstmt_info(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#if (RUNTIME_DEBUG_LEVEL >= 3)
    /* Print debug msg. */
    #define gpdebug(...) __DPF_PRINT_FUNC(DBG_LEVEL::DEBUG_, DPF_DEBUG_COLOR, "[DEBUG] ", __VA_ARGS__)
    /* Print continued debug msg. */
    #define gpdebug_c(...) __DPF_PRINT_FUNC(DBG_LEVEL::DEBUG_, DPF_DEBUG_COLOR, " . ", __VA_ARGS__)
    /* Print extended debug msg. */
    #define gpdebug_ex(...) __DPF_PRINT_FUNC(DBG_LEVEL::DEBUG_, DPF_DEBUG_COLOR, "", __VA_ARGS__)
    /* Statements for debug. */
    #define gstmt_debug(...) __DPF_DO_IF_RT_DBG(DBG_LEVEL::DEBUG_, __VA_ARGS__)
#else
    /* Print debug msg. */
    #define gpdebug(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print continued debug msg. */
    #define gpdebug_c(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print extended debug msg. */
    #define gpdebug_ex(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Statements for debug. */
    #define gstmt_debug(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#if (RUNTIME_DEBUG_LEVEL >= 4)
    /* Print msgdump msg. */
    #define gpmsgdump(...) __DPF_PRINT_FUNC(DBG_LEVEL::MSGDUMP, DPF_MSGDUMP_COLOR, "[MSGDUMP] ", __VA_ARGS__)
    /* Print continued msgdump msg. */
    #define gpmsgdump_c(...) __DPF_PRINT_FUNC(DBG_LEVEL::MSGDUMP, DPF_MSGDUMP_COLOR, " . ", __VA_ARGS__)
    /* Print extended msgdump msg. */
    #define gpmsgdump_ex(...) __DPF_PRINT_FUNC(DBG_LEVEL::MSGDUMP, DPF_MSGDUMP_COLOR, "", __VA_ARGS__)
    /* Statements for msgdump. */
    #define gstmt_msgdump(...) __DPF_DO_IF_RT_DBG(DBG_LEVEL::MSGDUMP, __VA_ARGS__)
#else
    /* Print msgdump msg. */
    #define gpmsgdump(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print continued msgdump msg. */
    #define gpmsgdump_c(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print extended msgdump msg. */
    #define gpmsgdump_ex(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Statements for msgdump. */
    #define gstmt_msgdump(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#if (RUNTIME_DEBUG_LEVEL >= 5)
    /* Print excessive msg. */
    #define gpexcessive(...) __DPF_PRINT_FUNC(DBG_LEVEL::EXCESSIVE, DPF_EXCESSIVE_COLOR, "[EXCESSIVE] ", __VA_ARGS__)
    /* Print continued excessive msg. */
    #define gpexcessive_c(...) __DPF_PRINT_FUNC(DBG_LEVEL::EXCESSIVE, DPF_EXCESSIVE_COLOR, " . ", __VA_ARGS__)
    /* Print extended excessive msg. */
    #define gpexcessive_ex(...) __DPF_PRINT_FUNC(DBG_LEVEL::EXCESSIVE, DPF_EXCESSIVE_COLOR, "", __VA_ARGS__)
    /* Statements for excessive. */
    #define gstmt_excessive(...) __DPF_DO_IF_RT_DBG(DBG_LEVEL::EXCESSIVE, __VA_ARGS__)
#else
    /* Print excessive msg. */
    #define gpexcessive(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print continued excessive msg. */
    #define gpexcessive_c(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Print extended excessive msg. */
    #define gpexcessive_ex(...) __DPF_DO_NOTHING(fmt::print(__VA_ARGS__))
    /* Statements for excessive. */
    #define gstmt_excessive(...) __DPF_DO_NOTHING(__VA_ARGS__)
#endif

#endif // __DEBUG_PRINT_FMT_H__
