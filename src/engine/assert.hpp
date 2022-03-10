#pragma once

#include <fmt/format.h>

#if defined(DC_ENABLE_ASSERT)

#define DC_ABORT() std::abort()

#define DC_DO_ASSERT(file, line, expr, msg, ...)                               \
  do                                                                           \
  {                                                                            \
    if (!(expr))                                                               \
    {                                                                          \
      const auto fmsg = fmt::format(msg, ##__VA_ARGS__);                       \
      fmt::print(stderr,                                                       \
                 "Failure in {} on line {}:\n\t{}\n\t" #expr "\n\n",           \
                 (file),                                                       \
                 (line),                                                       \
                 (fmsg));                                                      \
      DC_ABORT();                                                              \
    }                                                                          \
  } while (false)

#define DC_ASSERT(expr, msg, ...)                                              \
  DC_DO_ASSERT(__FILE__, __LINE__, expr, msg, ##__VA_ARGS__)

#define DC_DO_FAIL(file, line, msg, ...)                                       \
  do                                                                           \
  {                                                                            \
    const auto fmsg = fmt::format(msg, ##__VA_ARGS__);                         \
    fmt::print(stderr,                                                         \
               "Failure in {} on line {}:\n\t{}\n\n",                          \
               (file),                                                         \
               (line),                                                         \
               (fmsg));                                                        \
    DC_ABORT();                                                                \
  } while (false)

#define DC_FAIL(msg, ...) DC_DO_FAIL(__FILE__, __LINE__, msg, ##__VA_ARGS__)

#else

#define DC_DO_ASSERT(file, line, expr, msg) (void)0
#define DC_ASSERT(expr, msg)                (void)0
#define DC_DO_FAIL(file, line, msg)         (void)0
#define DC_FAIL(msg)                        (void)0

#endif
