#include "script_wrapper.hpp"
#include "assert.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "game_layer.hpp"
#include "log.hpp"
#include "mesh_asset.hpp"
#include "mesh_component.hpp"
#include "physic/character_controller_component.hpp"
#include "physic/physic_actor.hpp"
#include "physic/rigid_body.hpp"
#include "physic/rigid_body_component.hpp"
#include "physic/sphere_collider.hpp"
#include "physic/sphere_collider_component.hpp"
#include "profiling.hpp"
#include "scene_events.hpp"
#include "script/script_component.hpp"
#include "script_helper.hpp"
#include "window.hpp"

#include <mono/metadata/reflection.h>

#include <cstdint>
#include <memory>

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

dc::RigidBody *get_rigid_body(dc::Entity entity)
{
  if (entity.has_component<dc::RigidBodyComponent>())
  {
    const auto &component = entity.component<dc::RigidBodyComponent>();
    return component.physic_actor_;
  }

  DC_LOG_WARN("Can not find rigid body on entity");
  return nullptr;
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

  const auto has_component_func_iter = has_component_funcs.find(mono_type);
  if (has_component_func_iter != has_component_funcs.end())
  {
    return has_component_func_iter->second(entity);
  }
  else
  {
    DC_LOG_ERROR("No has component function found");
  }
  return false;
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

  const auto create_func_iter = create_component_funcs.find(mono_type);
  if (create_func_iter != create_component_funcs.end())
  {
    create_func_iter->second(entity);
  }
  else
  {
    DC_LOG_ERROR("No create component function found");
  }
}

void Dc_Entity_GetPosition(std::uint64_t entity_id, glm::vec3 *out_value)
{
  DC_ASSERT(out_value, "Out value not set");
  if (!out_value)
  {
    return;
  }

  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }

  *out_value = entity.position();
}

void Dc_Entity_SetPosition(std::uint64_t entity_id, glm::vec3 *value)
{
  DC_ASSERT(value, "Value not set");
  if (!value)
  {
    return;
  }

  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }

  if (entity.has_component<RigidBodyComponent>())
  {
    const auto physic_actor = get_rigid_body(entity);
    if (physic_actor)
    {
      physic_actor->set_translation(*value);
    }
  }
  else
  {
    entity.set_position(*value);
  }
}

void Dc_Entity_GetRotation(std::uint64_t entity_id, glm::vec3 *out_value)
{
  DC_ASSERT(out_value, "Out value not set");
  if (!out_value)
  {
    return;
  }

  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }

  *out_value = entity.rotation();
}

void Dc_Entity_SetRotation(std::uint64_t entity_id, glm::vec3 *value)
{
  DC_ASSERT(value, "Value not set");
  if (!value)
  {
    return;
  }

  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }

  if (entity.has_component<RigidBodyComponent>())
  {
    const auto physic_actor = get_rigid_body(entity);
    if (physic_actor)
    {
      physic_actor->set_rotation(*value);
    }
  }
  else
  {
    entity.set_rotation(*value);
  }
}

void Dc_Entity_GetScale(std::uint64_t entity_id, glm::vec3 *out_value)
{
  DC_ASSERT(out_value, "Out value not set");
  if (!out_value)
  {
    return;
  }

  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }

  *out_value = entity.scale();
}

void Dc_Entity_SetScale(std::uint64_t entity_id, glm::vec3 *value)
{
  DC_ASSERT(value, "Value not set");
  if (!value)
  {
    return;
  }

  auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    DC_LOG_WARN("Can not get entity from {}", entity_id);
    return;
  }
  entity.set_scale(*value);
}

bool Dc_Input_IsKeyPressed(Key key)
{
  return Engine::instance()->window()->key(key) == KeyAction::Press;
}

bool Dc_Input_IsMouseButtonPressed(MouseButton button)
{
  return Engine::instance()->window()->mouse_button(button) ==
         MouseButtonAction::Press;
}

std::uint64_t Dc_Scene_CreateEntity(MonoString *name)
{
  const auto scene =
      Engine::instance()->layer_stack()->layer<GameLayer>()->scene()->get();

  const auto entity = scene->create_entity(mono_string_to_string(name));
  return entity.id();
}

void Dc_Scene_RemoveEntity(std::uint64_t entity_id)
{
  const auto scene =
      Engine::instance()->layer_stack()->layer<GameLayer>()->scene()->get();

  scene->remove_entity(entity_id);
}

void Dc_MeshComponent_SetMesh(std::uint64_t entity_id, MonoString *mesh_name)
{
  const auto entity = get_entity(entity_id);
  if (!entity.has_component<MeshComponent>())
  {
    DC_LOG_WARN("Can not add mesh to an entity without a MeshComponent");
    return;
  }

  auto &mesh_component = entity.component<MeshComponent>();
  auto  asset          = Engine::instance()->asset_cache()->load_asset(
      Asset{mono_string_to_string(mesh_name)});
  mesh_component.model_ = std::dynamic_pointer_cast<MeshAssetHandle>(asset);
}

RigidBodyType Dc_RigidBodyComponent_GetBodyType(std::uint64_t entity_id)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return RigidBodyType::Static;
  }

  return entity.component<RigidBodyComponent>().body_type_;
}

void Dc_RigidBodyComponent_SetKinematic(std::uint64_t entity_id, bool value)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  auto &component = entity.component<RigidBodyComponent>();
  if (component.physic_actor_)
  {
    component.physic_actor_->set_kinematic(value);
  }
  else
  {
    component.is_kinematic_ = value;
  }
}

bool Dc_RigidBodyComponent_IsKinematic(std::uint64_t entity_id)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return false;
  }

  return entity.component<RigidBodyComponent>().is_kinematic_;
}

void Dc_RigidBodyComponent_SetMass(std::uint64_t entity_id, float value)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  auto &component = entity.component<RigidBodyComponent>();
  if (component.physic_actor_)
  {
    component.physic_actor_->set_mass(value);
  }
  else
  {
    component.mass_ = value;
  }
}

float Dc_RigidBodyComponent_GetMass(std::uint64_t entity_id)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return false;
  }

  return entity.component<RigidBodyComponent>().mass_;
}

void Dc_RigidBodyComponent_SetGravityDisabled(std::uint64_t entity_id,
                                              bool          value)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  auto &component = entity.component<RigidBodyComponent>();
  if (component.physic_actor_)
  {
    component.physic_actor_->set_gravity_disabled(value);
  }
  else
  {
    component.is_gravity_disabled_ = value;
  }
}

bool Dc_RigidBodyComponent_IsGravityDisabled(std::uint64_t entity_id)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return false;
  }

  return entity.component<RigidBodyComponent>().is_gravity_disabled_;
}

void Dc_RigidBodyComponent_SetLinearVelocity(std::uint64_t entity_id,
                                             glm::vec3    *value)
{
  if (!value)
  {
    DC_LOG_ERROR("Value not set");
    DC_FAIL("Value not set");
    return;
  }

  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }
  const auto rigid_body = entity.component<RigidBodyComponent>().physic_actor_;
  if (rigid_body)
  {
    rigid_body->set_linear_velocity(*value);
  }
  else
  {
    DC_LOG_WARN("Can not set velocity if no rigid body is set");
  }
}

void Dc_RigidBodyComponent_GetLinearVelocity(std::uint64_t entity_id,
                                             glm::vec3    *value)
{
  if (!value)
  {
    DC_LOG_ERROR("Value not set");
    DC_FAIL("Value not set");
    return;
  }

  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }
  const auto rigid_body = entity.component<RigidBodyComponent>().physic_actor_;
  if (rigid_body)
  {
    *value = rigid_body->get_linear_velocity();
    return;
  }
  else
  {
    DC_LOG_WARN("Can not get velocity if no rigid body is set");
    *value = glm::vec3{0.0f};
  }
}

void Dc_RigidBodyComponent_SetAngularVelocity(std::uint64_t entity_id,
                                              glm::vec3    *value)
{
  if (!value)
  {
    DC_LOG_ERROR("Value not set");
    DC_FAIL("Value not set");
    return;
  }

  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }
  const auto rigid_body = entity.component<RigidBodyComponent>().physic_actor_;
  if (rigid_body)
  {
    rigid_body->set_angular_velocity(*value);
  }
  else
  {
    DC_LOG_WARN("Can not set velocity if no rigid body is set");
  }
}

void Dc_RigidBodyComponent_GetAngularVelocity(std::uint64_t entity_id,
                                              glm::vec3    *value)
{
  if (!value)
  {
    DC_LOG_ERROR("Value not set");
    DC_FAIL("Value not set");
    return;
  }

  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }
  const auto rigid_body = entity.component<RigidBodyComponent>().physic_actor_;
  if (rigid_body)
  {
    *value = rigid_body->get_angular_velocity();
    return;
  }
  else
  {
    DC_LOG_WARN("Can not get velocity if no rigid body is set");
    *value = glm::vec3{0.0f};
  }
}

float Dc_SphereColliderComponent_GetRadius(std::uint64_t entity_id)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return 0;
  }

  return entity.component<SphereColliderComponent>().radius_;
}

void Dc_SphereColliderComponent_SetRadius(std::uint64_t entity_id, float value)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  auto &component = entity.component<SphereColliderComponent>();
  if (component.sphere_collider_)
  {
    component.sphere_collider_->set_radius(value);
  }
  else
  {
    component.radius_ = value;
  }
}

bool Dc_SphereColliderComponent_IsTrigger(std::uint64_t entity_id)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return false;
  }

  return entity.component<SphereColliderComponent>().is_trigger_;
}

void Dc_SphereColliderComponent_SetTrigger(std::uint64_t entity_id, bool value)
{
  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  auto &component = entity.component<SphereColliderComponent>();
  if (component.sphere_collider_)
  {
    component.sphere_collider_->set_trigger(value);
  }
  else
  {
    component.is_trigger_ = value;
  }
}

void Dc_SphereColliderComponent_SetOffset(std::uint64_t entity_id,
                                          glm::vec3    *value)
{
  if (!value)
  {
    DC_LOG_WARN("No value set");
    return;
  }

  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  auto &component = entity.component<SphereColliderComponent>();
  if (component.sphere_collider_)
  {
    component.sphere_collider_->set_offset(*value);
  }
  else
  {
    component.offset_ = *value;
  }
}

void Dc_SphereColliderComponent_GetOffset(std::uint64_t entity_id,
                                          glm::vec3    *value)
{
  if (!value)
  {
    DC_LOG_WARN("No value set");
    return;
  }

  const auto entity = get_entity(entity_id);
  if (!entity.valid())
  {
    return;
  }

  *value = entity.component<SphereColliderComponent>().offset_;
}

void Dc_ScriptComponent_SetScript(std::uint64_t entity_id,
                                  MonoString   *script_name)
{
  const auto entity = get_entity(entity_id);

  auto &component      = entity.component<ScriptComponent>();
  const auto &new_module_name = mono_string_to_string(script_name);
  if (new_module_name == component.module_name_)
  {
    return;
  }

  // TODO: This needs to be made better
  component.module_name_ = new_module_name;
  ComponentConstructEvent event{entity, ComponentType::Script};
  Engine::instance()->event_manager()->fire_event(event);
}

} // namespace dc::script_wrapper
