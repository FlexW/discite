#include "script_component.hpp"
#include "serialization.hpp"

namespace dc
{

void ScriptComponent::save(FILE *file) const
{
  write_string(file, module_name_);
}

void ScriptComponent::read(FILE *file) { read_string(file, module_name_); }

} // namespace dc
