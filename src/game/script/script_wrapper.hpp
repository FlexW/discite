#pragma once

#include "math.hpp"
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

} // namespace dc::script_wrapper
