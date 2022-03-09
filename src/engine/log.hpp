#pragma once

#include <spdlog/spdlog.h>

#define DC_LOG_DEBUG(msg, ...) SPDLOG_DEBUG(msg, ##__VA_ARGS__)
#define DC_LOG_INFO(msg, ...)  SPDLOG_INFO(msg, ##__VA_ARGS__)
#define DC_LOG_WARN(msg, ...)  SPDLOG_WARN(msg, ##__VA_ARGS__)
#define DC_LOG_ERROR(msg, ...) SPDLOG_ERROR(msg, ##__VA_ARGS__)
