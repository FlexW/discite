#pragma once

#include <filesystem>
#include <string>

std::filesystem::path
sanitize_file_path(const std::filesystem::path &file_path);
