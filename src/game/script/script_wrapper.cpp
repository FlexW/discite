#include "script_wrapper.hpp"
#include "log.hpp"
#include "profiling.hpp"

namespace dc::script_wrapper
{

void Dc_Log_LogMessage(LogLevel level, MonoString *message)
{
  DC_PROFILE_SCOPE("Dc_Log_LogMessage");

  const auto msg = mono_string_to_utf8(message);
  switch (level)
  {
  case LogLevel::Debug:
    DC_LOG_DEBUG("Script: {}", msg);
    break;
  case LogLevel::Info:
    DC_LOG_INFO("Script: {}", msg);
    break;
  case LogLevel::Warn:
    DC_LOG_WARN("Script: {}", msg);
    break;
  case LogLevel::Error:
    DC_LOG_ERROR("Script: {}", msg);
    break;
  }
}

} // namespace dc::script_wrapper
