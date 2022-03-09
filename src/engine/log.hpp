#pragma once

#include <sstream>

namespace dc
{

enum class LogLevel
{
  Debug   = 0,
  Info    = 1,
  Warning = 2,
  Error   = 3,
};

class Log
{
public:
  static LogLevel reporting_level();
  static void     set_reporting_level(LogLevel value);

  Log();
  ~Log();

  std::ostringstream &get(LogLevel level = LogLevel::Info);

private:
  static LogLevel level;

  LogLevel           message_level_ = LogLevel::Info;
  std::ostringstream os_;

  Log(const Log &) = delete;
  void operator=(const Log &) = delete;
};

} // namespace dc

#define DC_LOG_DEBUG() dc::Log().get(dc::LogLevel::Debug)
#define DC_LOG_INFO()  dc::Log().get(dc::LogLevel::Info)
#define DC_LOG_WARN()  dc::Log().get(dc::LogLevel::Warning)
#define DC_LOG_ERROR() dc::Log().get(dc::LogLevel::Error)
