#pragma once

#include "entity_script_instance.hpp"

#include <memory>
#include <string>

namespace dc
{

struct ScriptComponent
{
  std::string module_name_;

  std::unique_ptr<EntityScriptInstance> entity_script_{};
};

} // namespace dc
