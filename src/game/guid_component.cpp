#include "guid_component.hpp"
#include "serialization.hpp"
#include "uuid.hpp"

namespace dc
{

GuidComponent::GuidComponent() : id_{generate_uuid()} {}

GuidComponent::GuidComponent(Uuid uuid)
{
  if (uuid == 0)
  {
    // gen uuuid
    id_ = generate_uuid();
  }
  else
  {
    id_ = uuid;
  }
}

void GuidComponent::save(FILE *file) const { write_value(file, id_); }

void GuidComponent::read(FILE *file) { read_value(file, id_); }

} // namespace dc
