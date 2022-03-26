#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace dc
{

std::vector<std::uint8_t>
read_binary_file(const std::filesystem::path &file_path);
}
