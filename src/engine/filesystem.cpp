#include "filesystem.hpp"

#include <cstdio>
#include <fstream>
#include <ios>
#include <stdexcept>

namespace dc
{

std::vector<std::uint8_t>
read_binary_file(const std::filesystem::path &file_path)
{
  std::ifstream instream(file_path, std::ios::in | std::ios::binary);
  if (!instream.is_open())
  {
    throw std::runtime_error("Could not open " + file_path.string());
  }
  std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)),
                            std::istreambuf_iterator<char>());
  return data;
}

} // namespace dc
