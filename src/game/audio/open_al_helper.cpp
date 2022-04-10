#include "open_al_helper.hpp"
#include "log.hpp"

namespace dc
{

bool check_al_errors(const std::string &filename, const std::uint_fast32_t line)
{
  ALenum error = alGetError();
  if (error != AL_NO_ERROR)
  {
    std::string what;
    switch (error)
    {
    case AL_INVALID_NAME:
      what = "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL "
             "function";
      break;
    case AL_INVALID_ENUM:
      what = "AL_INVALID_ENUM: an invalid enum value was passed to an "
             "OpenAL function";
      break;
    case AL_INVALID_VALUE:
      what = "AL_INVALID_VALUE: an invalid value was passed to an OpenAL "
             "function";
      break;
    case AL_INVALID_OPERATION:
      what = "AL_INVALID_OPERATION: the requested operation is not valid";
      break;
    case AL_OUT_OF_MEMORY:
      what = "AL_OUT_OF_MEMORY: the requested operation resulted in "
             "OpenAL running out of memory";
      break;
    default:
      what = fmt::format("UNKNOWN AL ERROR: {}", error);
    }
    DC_LOG_ERROR("OpenAL error: {}:{} {}", filename, line, what);
    return false;
  }
  return true;
}

bool check_alc_errors(const std::string       &filename,
                      const std::uint_fast32_t line,
                      ALCdevice               *device)
{
  ALCenum error = alcGetError(device);
  if (error != ALC_NO_ERROR)
  {
    std::string what;
    switch (error)
    {
    case ALC_INVALID_VALUE:
      what = "ALC_INVALID_VALUE: an invalid value was passed to an "
             "OpenAL function";
      break;
    case ALC_INVALID_DEVICE:
      what = "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL "
             "function";
      break;
    case ALC_INVALID_CONTEXT:
      what = "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL "
             "function";
      break;
    case ALC_INVALID_ENUM:
      what = "ALC_INVALID_ENUM: an unknown enum value was passed to an "
             "OpenAL function";
      break;
    case ALC_OUT_OF_MEMORY:
      what = "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an "
             "OpenAL function";
      break;
    default:
      what = fmt::format("UNKNOWN ALC ERROR: {}", error);
    }
    DC_LOG_ERROR("OpenAL Context error: {}:{} {}", filename, line, what);
    return false;
  }
  return true;
}
} // namespace dc
