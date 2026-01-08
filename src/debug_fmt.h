#ifndef __DEBUG_PRINT_FMT_H__
#define __DEBUG_PRINT_FMT_H__

// #define DEBUG

#ifdef DEBUG
    #include "libs/fmt/color.h"
    #include "libs/fmt/core.h"

    #define gprint(...)                                                                                                \
        do {                                                                                                           \
            fmt::print(stderr, fmt::fg(fmt::color::green), "[DEBUG] ");                                                \
            fmt::print(stderr, __VA_ARGS__);                                                                           \
        } while (0)

    #define gprintc(...)                                                                                               \
        do {                                                                                                           \
            fmt::print(stderr, fmt::fg(fmt::color::green), " >      ");                                                \
            fmt::print(stderr, __VA_ARGS__);                                                                           \
        } while (0)

    #define gstmt(...)                                                                                                 \
        do {                                                                                                           \
            __VA_ARGS__;                                                                                               \
        } while (0)

    #include <cassert>

    #define grequire(...) assert(__VA_ARGS__)
    #define gassert(...) assert(__VA_ARGS__)
    #define gensure(...) assert(__VA_ARGS__)

#else
    /* Not enabled. */
    #define gprint(...)
    /* Not enabled. */
    #define gprintc(...)
    /* Not enabled. */
    #define gstmt(...)
    /* Not enabled. */
    #define grequire(...)
    /* Not enabled. */
    #define gassert(...)
    /* Not enabled. */
    #define gensure(...)
#endif

#endif // __DEBUG_PRINT_FMT_H__
