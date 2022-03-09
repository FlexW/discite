#pragma once

#include <filesystem>
#include <string>

namespace dc
{

std::filesystem::path
sanitize_file_path(const std::filesystem::path &file_path);
}
