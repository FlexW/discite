#pragma once

#include <filesystem>
#include <type_traits>

namespace dc
{

template <typename T> constexpr auto to_underlying(T r) noexcept
{
  return static_cast<std::underlying_type_t<T>>(r);
}

std::filesystem::path executable_path();

} // namespace dc
