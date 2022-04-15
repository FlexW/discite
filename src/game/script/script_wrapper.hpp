#pragma once

#include "math.hpp"
#include "physic/rigid_body.hpp"
#include "window.hpp"

#include <mono/metadata/object.h>

#include <cstdint>

namespace dc::script_wrapper
{
enum class LogLevel : std::int32_t
{
  Debug = 1 << 0,
  Info  = 1 << 1,
  Warn  = 1 << 2,
  Error = 1 << 3,
};

void Dc_Log_LogMessage(LogLevel level, MonoString *message);

bool Dc_Entity_HasComponent(std::uint64_t entity_id, void *type);
void Dc_Entity_CreateComponent(std::uint64_t entity_id, void *type);
void Dc_Entity_GetPosition(std::uint64_t entity_id, glm::vec3 *out_value);
void Dc_Entity_SetPosition(std::uint64_t entity_id, glm::vec3 *value);
void Dc_Entity_GetRotation(std::uint64_t entity_id, glm::vec3 *out_value);
void Dc_Entity_SetRotation(std::uint64_t entity_id, glm::vec3 *value);
void Dc_Entity_GetScale(std::uint64_t entity_id, glm::vec3 *out_value);
void Dc_Entity_SetScale(std::uint64_t entity_id, glm::vec3 *value);

bool Dc_Input_IsKeyPressed(Key key);
bool Dc_Input_IsMouseButtonPressed(MouseButton button);

std::uint64_t Dc_Scene_CreateEntity(MonoString *name);
void          Dc_Scene_RemoveEntity(std::uint64_t entity_id);

void Dc_MeshComponent_SetMesh(std::uint64_t entity_id, MonoString *mesh_name);

void Dc_ScriptComponent_SetScript(std::uint64_t entity_id,
                                  MonoString   *script_name);

RigidBodyType Dc_RigidBodyComponent_GetBodyType(std::uint64_t entity_id);
void  Dc_RigidBodyComponent_SetKinematic(std::uint64_t entity_id, bool value);
bool  Dc_RigidBodyComponent_IsKinematic(std::uint64_t entity_id);
void  Dc_RigidBodyComponent_SetMass(std::uint64_t entity_id, float value);
float Dc_RigidBodyComponent_GetMass(std::uint64_t entity_id);
void  Dc_RigidBodyComponent_SetGravityDisabled(std::uint64_t entity_id,
                                               bool          value);
bool  Dc_RigidBodyComponent_IsGravityDisabled(std::uint64_t entity_id);
void  Dc_RigidBodyComponent_SetLinearVelocity(std::uint64_t entity_id,
                                              glm::vec3    *value);
void  Dc_RigidBodyComponent_GetLinearVelocity(std::uint64_t entity_id,
                                              glm::vec3    *value);
void  Dc_RigidBodyComponent_SetAngularVelocity(std::uint64_t entity_id,
                                               glm::vec3    *value);
void  Dc_RigidBodyComponent_GetAngularVelocity(std::uint64_t entity_id,
                                               glm::vec3    *value);

float Dc_SphereColliderComponent_GetRadius(std::uint64_t entity_id);
void Dc_SphereColliderComponent_SetRadius(std::uint64_t entity_id, float value);
bool  Dc_SphereColliderComponent_IsTrigger(std::uint64_t entity_id);
void Dc_SphereColliderComponent_SetTrigger(std::uint64_t entity_id, bool value);
void  Dc_SphereColliderComponent_SetOffset(std::uint64_t entity_id,
                                           glm::vec3    *value);
void  Dc_SphereColliderComponent_GetOffset(std::uint64_t entity_id,
                                           glm::vec3    *value);

} // namespace dc::script_wrapper
