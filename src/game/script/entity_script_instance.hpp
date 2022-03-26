#pragma once

#include "entity_script_type.hpp"
#include "uuid.hpp"

#include <mono/metadata/object-forward.h>

#include <cstdint>
#include <memory>

namespace dc
{

class EntityScriptInstance
{
public:
  EntityScriptInstance(Uuid                              id,
                       MonoDomain                       *domain,
                       std::shared_ptr<EntityScriptType> type);

  void construct();
  void on_update(float delta_time);

private:
  std::shared_ptr<EntityScriptType> type_{};

  Uuid          id_;
  std::uint32_t handle_{};

  MonoObject *mono_instance();
};

} // namespace dc
