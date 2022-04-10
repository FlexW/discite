#include "asset_description.hpp"
#include "serialization.hpp"

namespace dc
{

void AssetDescription::write(FILE *file) const
{
  assert(file);
  write_value(file, magic_value_);
  write_value(file, version_);
  write_string(file, original_file_);
}

void AssetDescription::write(std::ofstream &file) const
{
  write_value(file, magic_value_);
  write_value(file, version_);
  write_string(file, original_file_);
}

void AssetDescription::read(FILE *file)
{
  assert(file);
  read_value(file, magic_value_);
  read_value(file, version_);
  read_string(file, original_file_);
}

void AssetDescription::read(std::ifstream &file)
{
  read_value(file, magic_value_);
  read_value(file, version_);
  read_string(file, original_file_);
}

} // namespace dc
