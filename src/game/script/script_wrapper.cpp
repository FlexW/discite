#include "script_wrapper.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "game_layer.hpp"
#include "log.hpp"
#include "profiling.hpp"

#include <cstdint>
#include <mono/metadata/reflection.h>

extern std::unordered_map<MonoType *, std::function<bool(dc::Entity &)>>
    has_component_funcs;
extern std::unordered_map<MonoType *, std::function<void(dc::Entity &)>>
    create_component_funcs;

namespace
{

dc::Entity get_entity(std::uint64_t entity_id)
{
  const auto game_layer =
      dc::Engine::instance()->layer_stack()->layer<dc::GameLayer>();
  if (!game_layer)
  {
    DC_LOG_WARN("Can not get entity without game layer");
    return {};
  }

  const auto scene = game_layer->scene()->get();
  if (!scene)
  {
    DC_LOG_WARN("Can not get entity withoug active scene");
    return {};
  }

  return scene->entity(entity_id);
}

} // namespace

namespace dc::script_wrapper
{

void Dc_Log_LogMessage(LogLevel level, MonoString *message)
{
  DC_PROFILE_SCOPE("Dc_Log_LogMessage");

  const auto msg = mono_string_to_utf8(message);
  switch (level)
  {
  case LogLevel::Debug:
    DC_LOG_DEBUG("Script: {}", msg);
    break;
  case LogLevel::Info:
    DC_LOG_INFO("Script: {}", msg);
    break;
  case LogLevel::Warn:
    DC_LOG_WARN("Script: {}", msg);
    break;
  case LogLevel::Error:
    DC_LOG_ERROR("Script: {}", msg);
    break;
  }
}

bool Dc_Entity_HasComponent(std::uint64_t entity_id, void *type)
{
  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return false;
  }

  const auto mono_type =
      mono_reflection_type_get_type(static_cast<MonoReflectionType *>(type));
  return has_component_funcs[mono_type](entity);
}

void Dc_Entity_CreateComponent(std::uint64_t entity_id, void *type)
{
  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }

  const auto mono_type =
      mono_reflection_type_get_type(static_cast<MonoReflectionType *>(type));
  create_component_funcs[mono_type](entity);
}

} // namespace dc::script_wrapper
