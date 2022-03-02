#include "guid_component.hpp"
#include "serialization.hpp"

#include <random>

namespace
{

std::random_device                      random_device;
std::mt19937_64                         eng(random_device());
std::uniform_int_distribution<uint64_t> uniform_distribution;

} // namespace

GuidComponent::GuidComponent() : id_{uniform_distribution(eng)} {}

GuidComponent::GuidComponent(Uuid uuid)
{
  if (uuid == 0)
  {
    // gen uuuid
    id_ = uniform_distribution(eng);
  }
  else
  {
    id_ = uuid;
  }
}

void GuidComponent::save(FILE *file) const { write_value(file, id_); }

void GuidComponent::read(FILE *file) { read_value(file, id_); }
