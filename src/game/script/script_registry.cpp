#include "script_registry.hpp"
#include "entity.hpp"
#include "log.hpp"
#include "name_component.hpp"
#include "script_wrapper.hpp"
#include "transform_component.hpp"

#include <mono/metadata/object-forward.h>
#include <mono/metadata/reflection.h>

#include <functional>
#include <unordered_map>

std::unordered_map<MonoType *, std::function<bool(dc::Entity &)>>
    has_component_funcs;
std::unordered_map<MonoType *, std::function<void(dc::Entity &)>>
    create_component_funcs;

#define MONO_METHOD(m) reinterpret_cast<void *>(m)

#define REGISTER_COMPONENT_TYPE(Type, core_assembly_image)                     \
  {                                                                            \
    std::string str{"Dc." #Type};                                              \
    const auto  type =                                                         \
        mono_reflection_type_from_name(str.data(), core_assembly_image);       \
    if (type)                                                                  \
    {                                                                          \
      has_component_funcs[type] = [](Entity &entity)                           \
      { return entity.has_component<Type>(); };                                \
      create_component_funcs[type] = [](Entity &entity)                        \
      { entity.add_component<Type>(); };                                       \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      DC_LOG_WARN("No Script component class found for " #Type "!");           \
    }                                                                          \
  }

namespace dc::script_registry
{

void register_all(MonoImage *core_image)
{
  REGISTER_COMPONENT_TYPE(NameComponent, core_image);
  REGISTER_COMPONENT_TYPE(TransformComponent, core_image);

  // entity
  mono_add_internal_call(
      "Dc.Entity::CreateComponent_Native",
      MONO_METHOD(dc::script_wrapper::Dc_Entity_CreateComponent));
  mono_add_internal_call(
      "Dc.Entity::HasComponent_Native",
      MONO_METHOD(dc::script_wrapper::Dc_Entity_HasComponent));

  // logging
  mono_add_internal_call("Dc.Log::LogMessage_Native",
                         MONO_METHOD(dc::script_wrapper::Dc_Log_LogMessage));
}

} // namespace dc::script_registry
