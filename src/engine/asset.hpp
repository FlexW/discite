#pragma once

#include <cstdint>
#include <string>

namespace dc
{

class Asset
{
public:
  Asset() = default;
  explicit Asset(const std::string &name);

  std::string id() const;
  std::string type() const;

private:
  std::string id_{};
  std::string type_{};
};

} // namespace dc
