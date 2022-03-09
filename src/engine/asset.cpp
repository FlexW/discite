#include "asset.hpp"

#include <filesystem>

namespace dc
{

Asset::Asset(const std::string &name)
{
  id_   = name;
  type_ = std::filesystem::path(name).extension().string();
}

std::string Asset::id() const { return id_; }

std::string Asset::type() const { return type_; }

} // namespace dc
