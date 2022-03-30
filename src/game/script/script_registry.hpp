#pragma once

#include <mono/metadata/object-forward.h>

namespace dc::script_registry
{

void register_all(MonoImage *core_image);
}
