#pragma once

#include <fmt/format.h>

#if defined(DC_ENABLE_ASSERT)

#define DC_SEGV                                                                \
  *(volatile int *)0 = 0;                                                      \
  std::exit(EXIT_FAILURE);

#define DC_DO_ASSERT(file, line, expr, msg, ...)                               \
  do                                                                           \
  {                                                                            \
    if (!(expr))                                                               \
    {                                                                          \
      fmt::print(stderr,                                                       \
                 "Failure in {} on line {}:\n\n" #expr "\n\n{}\n",             \
                 (file),                                                       \
                 (line),                                                       \
                 (msg),                                                        \
                 ##__VA_ARGS__);                                               \
      DC_SEGV                                                                  \
    }                                                                          \
  } while (false)

#define DC_ASSERT(expr, msg, ...)                                              \
  DC_DO_ASSERT(__FILE__, __LINE__, expr, msg, ##__VA_ARGS__)

#define DC_DO_FAIL(file, line, msg, ...)                                       \
  do                                                                           \
  {                                                                            \
    fmt::print(stderr,                                                         \
               "Failure in {} on line {}:\n\n{}\n",                            \
               (file),                                                         \
               (line),                                                         \
               (msg),                                                          \
               ##__VA_ARGS__);                                                 \
    DC_SEGV                                                                    \
  } while (false)

#define DC_FAIL(msg, ...) DC_DO_FAIL(__FILE__, __LINE__, msg, ##__VA_ARGS__)

#else

#define DC_DO_ASSERT(file, line, expr, msg)
#define DC_ASSERT(expr, msg)
#define DC_DO_FAIL(file, line, msg)
#define DC_FAIL(msg)

#endif
