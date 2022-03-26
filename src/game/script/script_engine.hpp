#pragma once

#include "entity.hpp"
#include "entity_script_instance.hpp"
#include "entity_script_type.hpp"

#include <memory>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <filesystem>
#include <unordered_map>

namespace dc
{

class ScriptEngine
{
public:
  ScriptEngine();
  ~ScriptEngine();

  void load_game_assembly(const std::filesystem::path &file_path);

  void init_entity(Entity entity);

  void on_update(float delta_time);

  std::unique_ptr<EntityScriptInstance>
  construct_entity(const Entity &entity, const std::string &module_name);

private:
  std::unordered_map<std::string, std::shared_ptr<EntityScriptType>>
      entity_script_types_;

  static std::string get_namespace_name(const std::string &module_name);
  static std::string get_class_name(const std::string &module_name);

  std::shared_ptr<EntityScriptType>
  entity_script_type(const std::string &module_name);

  std::filesystem::path core_assembly_file_path_;

  MonoDomain *current_domain_{};

  MonoAssembly *game_assembly_{};
  MonoImage    *game_assembly_image_{};

  MonoAssembly *core_assembly_{};
  MonoImage    *core_assembly_image_{};
  MonoClass    *entity_class_{};

  void init_mono();
  void shutdown_mono();

  bool is_valid_module(const std::string &module_name);

  void load_core_assembly(const std::filesystem::path &file_path);
};

} // namespace dc
