#include "importer.hpp"

#include <cctype>

namespace dc
{

std::filesystem::path sanitize_file_path(const std::filesystem::path &file_path)
{
  std::string file_path_str{file_path.parent_path() / file_path.stem()};

  for (std::size_t i{0}; i < file_path_str.size(); ++i)
  {
    auto character = file_path_str[i];
    if (character == '[' || character == ']' || character == '-' ||
        character == '.')
    {
      character = '_';
    }
    file_path_str[i] = std::tolower(character);
  }

  return (file_path_str + file_path.extension().string());
}

} // namespace dc
