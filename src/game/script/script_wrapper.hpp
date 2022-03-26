#pragma once

#include <mono/metadata/object.h>

#include <cstdint>

namespace dc::script_wrapper
{
enum class LogLevel : std::int32_t
{
  Debug = 1 << 0,
  Info  = 1 << 1,
  Warn  = 1 << 2,
  Error = 1 << 3,
};

void Dc_Log_LogMessage(LogLevel level, MonoString *message);

} // namespace dc::script
