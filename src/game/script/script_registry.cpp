#include "script_registry.hpp"
#include "audio/audio_listener_component.hpp"
#include "audio/audio_source_component.hpp"
#include "directional_light_component.hpp"
#include "entity.hpp"
#include "log.hpp"
#include "mesh_component.hpp"
#include "name_component.hpp"
#include "physic/box_collider_component.hpp"
#include "physic/capsule_collider_component.hpp"
#include "physic/character_controller_component.hpp"
#include "physic/mesh_collider_component.hpp"
#include "physic/rigid_body_component.hpp"
#include "physic/sphere_collider_component.hpp"
#include "point_light_component.hpp"
#include "script/script_component.hpp"
#include "script_wrapper.hpp"
#include "skinned_mesh_component.hpp"
#include "sky_component.hpp"
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

#define REGISTER_FUNCTION(module, name)                                        \
  mono_add_internal_call(                                                      \
      "Dc." #module "::" #name "_Native",                                      \
      MONO_METHOD(dc::script_wrapper::Dc_##module##_##name))

namespace dc::script_registry
{

void register_all(MonoImage *core_image)
{
  REGISTER_COMPONENT_TYPE(NameComponent, core_image);
  REGISTER_COMPONENT_TYPE(TransformComponent, core_image);
  REGISTER_COMPONENT_TYPE(MeshComponent, core_image);
  REGISTER_COMPONENT_TYPE(SkinnedMeshComponent, core_image);
  REGISTER_COMPONENT_TYPE(DirectionalLightComponent, core_image);
  REGISTER_COMPONENT_TYPE(PointLightComponent, core_image);
  REGISTER_COMPONENT_TYPE(SkyComponent, core_image);
  REGISTER_COMPONENT_TYPE(ScriptComponent, core_image);
  REGISTER_COMPONENT_TYPE(CharacterControllerComponent, core_image);
  REGISTER_COMPONENT_TYPE(RigidBodyComponent, core_image);
  REGISTER_COMPONENT_TYPE(BoxColliderComponent, core_image);
  REGISTER_COMPONENT_TYPE(SphereColliderComponent, core_image);
  REGISTER_COMPONENT_TYPE(CapsuleColliderComponent, core_image);
  REGISTER_COMPONENT_TYPE(MeshColliderComponent, core_image);
  REGISTER_COMPONENT_TYPE(AudioSourceComponent, core_image);
  REGISTER_COMPONENT_TYPE(AudioListenerComponent, core_image);

  // scene
  REGISTER_FUNCTION(Scene, CreateEntity);
  REGISTER_FUNCTION(Scene, RemoveEntity);

  // entity
  REGISTER_FUNCTION(Entity, CreateComponent);
  REGISTER_FUNCTION(Entity, HasComponent);
  REGISTER_FUNCTION(Entity, GetPosition);
  REGISTER_FUNCTION(Entity, SetPosition);
  REGISTER_FUNCTION(Entity, GetRotation);
  REGISTER_FUNCTION(Entity, SetRotation);
  REGISTER_FUNCTION(Entity, GetScale);
  REGISTER_FUNCTION(Entity, SetScale);

  // mesh component
  REGISTER_FUNCTION(MeshComponent, SetMesh);

  // script component
  REGISTER_FUNCTION(ScriptComponent, SetScript);

  // rigid body component
  REGISTER_FUNCTION(RigidBodyComponent, GetBodyType);
  REGISTER_FUNCTION(RigidBodyComponent, GetMass);
  REGISTER_FUNCTION(RigidBodyComponent, SetMass);
  REGISTER_FUNCTION(RigidBodyComponent, IsKinematic);
  REGISTER_FUNCTION(RigidBodyComponent, SetKinematic);
  REGISTER_FUNCTION(RigidBodyComponent, IsGravityDisabled);
  REGISTER_FUNCTION(RigidBodyComponent, SetGravityDisabled);
  REGISTER_FUNCTION(RigidBodyComponent, SetLinearVelocity);
  REGISTER_FUNCTION(RigidBodyComponent, GetLinearVelocity);
  REGISTER_FUNCTION(RigidBodyComponent, SetAngularVelocity);
  REGISTER_FUNCTION(RigidBodyComponent, GetAngularVelocity);

  // sphere collider component
  REGISTER_FUNCTION(SphereColliderComponent, GetRadius);
  REGISTER_FUNCTION(SphereColliderComponent, SetRadius);
  REGISTER_FUNCTION(SphereColliderComponent, GetOffset);
  REGISTER_FUNCTION(SphereColliderComponent, SetOffset);
  REGISTER_FUNCTION(SphereColliderComponent, IsTrigger);
  REGISTER_FUNCTION(SphereColliderComponent, SetTrigger);

  // logging
  REGISTER_FUNCTION(Log, LogMessage);

  // input
  REGISTER_FUNCTION(Input, IsKeyPressed);
  REGISTER_FUNCTION(Input, IsMouseButtonPressed);
}

} // namespace dc::script_registry
