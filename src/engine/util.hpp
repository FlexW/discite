#pragma once

#include <type_traits>

namespace dc
{

template <typename T> constexpr auto to_underlying(T r) noexcept
{
  return static_cast<std::underlying_type_t<T>>(r);
}

} // namespace dc
