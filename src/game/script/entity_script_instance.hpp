#pragma once

#include "entity.hpp"
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
  void on_create();
  void on_update(float delta_time);
  void on_collison_begin(Entity collidee);
  void on_collison_end(Entity collidee);
  void on_trigger_begin(Entity collidee);
  void on_trigger_end(Entity collidee);

private:
  std::shared_ptr<EntityScriptType> type_{};

  Uuid          id_;
  std::uint32_t handle_{};

  MonoObject *mono_instance();
};

} // namespace dc
