#include "util.hpp"
#include "log.hpp"

#ifdef linux
#include <unistd.h>
#endif

#include <array>

namespace dc
{

std::filesystem::path executable_path()
{
#ifdef linux
  std::array<char, 250> buffer{};
  const auto            result =
      readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
  if (result == -1)
  {
    DC_LOG_ERROR("Could not read executable path");
    return {};
  }

  return buffer.data();
#else
#error Not implemented
#endif
}

} // namespace dc
