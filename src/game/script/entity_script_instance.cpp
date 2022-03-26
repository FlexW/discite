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

void EntityScriptInstance::on_update(float delta_time)
{
  type_->on_update(mono_instance(), delta_time);
}

} // namespace dc
