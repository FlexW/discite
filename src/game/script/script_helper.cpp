#include "script_helper.hpp"
#include "assert.hpp"
#include "log.hpp"

#include <mono/metadata/object.h>
#include <mono/utils/mono-error.h>

namespace dc
{
std::string mono_string_to_string(MonoString *mono_string)
{
  if (mono_string == nullptr || mono_string_length(mono_string) == 0)
  {
    return {};
  }

  MonoError error;
  const auto utf8 = mono_string_to_utf8_checked(mono_string, &error);
  if (check_mono_error(error))
  {
    return "";
  }

  std::string result{utf8};
  mono_free(utf8);

  return result;
}

bool check_mono_error(MonoError &error)
{
  const auto has_error = !mono_error_ok(&error);

  if (has_error)
  {
    const auto error_code    = mono_error_get_error_code(&error);
    const auto error_message = mono_error_get_message(&error);

    DC_LOG_ERROR("Mono error: {} {}", error_code, error_message);
    mono_error_cleanup(&error);
    DC_FAIL("Mono error: {} {}", error_code, error_message);
  }

  return has_error;
}

} // namespace dc
