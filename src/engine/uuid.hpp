#pragma once

#include <cstdint>

namespace dc
{

using Uuid = std::uint64_t;

Uuid generate_uuid();
}
