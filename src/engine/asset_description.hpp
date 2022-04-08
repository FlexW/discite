#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

namespace dc
{

struct AssetDescription
{
  std::uint32_t magic_value_{0xdeadbeef};
  std::uint32_t version_{0};
  std::string   original_file_;

  void write(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
