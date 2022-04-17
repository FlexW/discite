#include "entity_script_instance.hpp"
#include "assert.hpp"
#include "log.hpp"
#include "uuid.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

namespace dc
{

EntityScriptInstance::EntityScriptInstance(
    Uuid                              id,
    MonoDomain                       *domain,
    std::shared_ptr<EntityScriptType> type)
    : type_{type},
      id_{id}
{
  DC_ASSERT(type_, "Type needs to be non nullptr");

  const auto instance = mono_object_new(domain, type_->mono_class_);
  if (!instance)
  {
    DC_LOG_WARN("Could not create mono object");
    return;
  }

  mono_runtime_object_init(instance);
  handle_ = mono_gchandle_new(instance, false);
}

MonoObject *EntityScriptInstance::mono_instance()
{
  return mono_gchandle_get_target(handle_);
}

void EntityScriptInstance::construct()
{
  type_->construct(mono_instance(), id_);
}

void EntityScriptInstance::on_create() { type_->on_create(mono_instance()); }

void EntityScriptInstance::on_update(float delta_time)
{
  type_->on_update(mono_instance(), delta_time);
}

void EntityScriptInstance::on_collison_begin(Entity collidee)
{
  type_->on_collision_begin(mono_instance(), collidee);
}

void EntityScriptInstance::on_collison_end(Entity collidee)
{
  type_->on_collision_end(mono_instance(), collidee);
}

void EntityScriptInstance::on_trigger_begin(Entity collidee)
{
  type_->on_trigger_begin(mono_instance(), collidee);
}

void EntityScriptInstance::on_trigger_end(Entity collidee)
{
  type_->on_trigger_end(mono_instance(), collidee);
}

void EntityScriptInstance::on_key_press(Key key)
{
  type_->on_key_press(mono_instance(), key);
}

void EntityScriptInstance::on_key_release(Key key)
{
  type_->on_key_release(mono_instance(), key);
}

} // namespace dc
