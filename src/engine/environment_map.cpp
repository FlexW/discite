#include "environment_map.hpp"

namespace dc
{

EnvironmentMap::EnvironmentMap(const std::string &name, std::vector<std::uint8_t> data)
    : name_{name},
      data_{std::move(data)}
{
}

std::string EnvironmentMap::name() const { return name_; }

std::vector<std::uint8_t> EnvironmentMap::data() const { return data_; }

void EnvironmentMap::release_data() { data_ = {}; }

} // namespace dc
