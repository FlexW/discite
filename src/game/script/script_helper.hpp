#pragma once

#include <mono/metadata/object.h>
#include <mono/utils/mono-error.h>

#include <string>

namespace dc
{

std::string mono_string_to_string(MonoString *mono_string);

bool check_mono_error(MonoError &error);
}
