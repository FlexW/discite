#include "script_engine.hpp"
#include "assert.hpp"
#include "entity.hpp"
#include "entity_script_instance.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "script_registry.hpp"
#include "string.hpp"
#include "util.hpp"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/image.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace
{
void dump_image(const std::string &assembly, MonoImage *image)
{
  DC_LOG_DEBUG("Dump {}", assembly);

  const auto table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
  const auto rows       = mono_table_info_get_rows(table_info);

  // for each row, get some of its values
  for (int i = 0; i < rows; i++)
  {
    uint32_t cols[MONO_TYPEDEF_SIZE];
    mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
    const auto class_name =
        mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
    const auto namespace_name =
        mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);

    DC_LOG_DEBUG("Class: {}.{}", namespace_name, class_name);

    const auto mono_class =
        mono_class_from_name(image, namespace_name, class_name);

    void       *iter = NULL;
    while (true)
    {
      const auto method = mono_class_get_methods(mono_class, &iter);
      if (!method)
      {
        break;
      }
      const auto method_name = mono_method_full_name(method, 1);
      DC_LOG_DEBUG("Method: {}", method_name);
    }
  }
}
} // namespace

namespace dc
{

std::string ScriptEngine::get_namespace_name(const std::string &module_name)
{
  if (contains(module_name, '.'))
  {
    return module_name.substr(0, module_name.find_last_of('.'));
  }
  return {};
}

std::string ScriptEngine::get_class_name(const std::string &module_name)
{
  if (contains(module_name, '.'))
  {
    return module_name.substr(module_name.find_last_of('.') + 1);
  }
  return module_name;
}

MonoAssembly *load_assembly_from_file(const std::filesystem::path &file_path)
{
  DC_LOG_INFO("Load assembly {}", file_path.string());

  try
  {
    auto data = read_binary_file(file_path);

    MonoImageOpenStatus status{};
    const auto          image =
        mono_image_open_from_data_full(reinterpret_cast<char *>(data.data()),
                                       static_cast<std::uint32_t>(data.size()),
                                       1,
                                       &status,
                                       0);
    if (status != MONO_IMAGE_OK)
    {
      DC_LOG_WARN("Could not load mono image from {}", file_path.string());
      return nullptr;
    }

    const auto assembly =
        mono_assembly_load_from_full(image,
                                     file_path.string().c_str(),
                                     &status,
                                     0);
    mono_image_close(image);

    if (status != MONO_IMAGE_OK)
    {
      DC_LOG_WARN("Could not load mono assembly from {}", file_path.string());
      return nullptr;
    }

    return assembly;
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not read mono file from {}", file_path.string());
    return nullptr;
  }
}

MonoImage *assembly_image(MonoAssembly *assembly)
{
  auto image = mono_assembly_get_image(assembly);
  if (!image)
  {
    DC_LOG_WARN("Could not load image from assembly");
  }
  return image;
}

void register_types() { script_registry::register_all(); }

} // namespace dc

namespace dc
{

ScriptEngine::ScriptEngine()
{
  init_mono();
  const auto app_file_path = executable_path();
  load_core_assembly(app_file_path.parent_path() / "DisciteRuntime.dll");
}

ScriptEngine::~ScriptEngine()
{
  // Apparently according to
  // https://www.mono-project.com/docs/advanced/embedding/ we can't do
  // mono_jit_init in the same process after mono_jit_cleanup... so don't do
  // this mono_jit_cleanup(s_MonoDomain);
}

void ScriptEngine::init_mono() { mono_jit_init("discite"); }

void ScriptEngine::load_game_assembly(const std::filesystem::path &file_path)
{
  DC_ASSERT(!game_assembly_, "Already loaded");

  const auto game_assembly       = load_assembly_from_file(file_path);
  const auto game_assembly_image = assembly_image(game_assembly);
  register_types();

  game_assembly_       = game_assembly;
  game_assembly_image_ = game_assembly_image;

  dump_image(file_path.string(), game_assembly_image_);
}

void ScriptEngine::load_core_assembly(const std::filesystem::path &file_path)
{
  DC_ASSERT(!current_domain_, "Already loaded");

  core_assembly_file_path_ = file_path;

  std::string domain_name{"Discite Runtime"};
  current_domain_ = mono_domain_create_appdomain(domain_name.data(), nullptr);
  mono_domain_set(current_domain_, false);

  core_assembly_ = load_assembly_from_file(core_assembly_file_path_);
  if (!core_assembly_)
  {
    DC_LOG_WARN("Could not load core assembly {}", file_path.string());
    return;
  }

  core_assembly_image_ = assembly_image(core_assembly_);
  entity_class_ = mono_class_from_name(core_assembly_image_, "Dc", "Entity");
}

bool ScriptEngine::is_valid_module(const std::string &module_name)
{
  if (!game_assembly_image_)
  {
    DC_LOG_WARN("Can not check if {} is a valid module since game assembly is "
                "not loaded",
                module_name);
    return false;
  }

  const auto namespace_name = get_namespace_name(module_name);
  const auto class_name     = get_class_name(module_name);

  const auto mono_class = mono_class_from_name(game_assembly_image_,
                                               namespace_name.c_str(),
                                               class_name.c_str());
  if (!mono_class)
  {
    return false;
  }

  const auto is_entity_subclass =
      mono_class_is_subclass_of(mono_class, entity_class_, 0);

  if (!is_entity_subclass)
  {
    DC_LOG_WARN("{} is not a sub class of Entity", module_name);
  }

  return is_entity_subclass;
}

std::unique_ptr<EntityScriptInstance>
ScriptEngine::construct_entity(const Entity      &entity,
                               const std::string &module_name)
{
  if (!game_assembly_image_)
  {
    DC_LOG_WARN("Can not construct entity script without game assembly");
    return {};
  }

  const auto type = entity_script_type(module_name);
  if (!type)
  {
    return {};
  }

  auto instance = std::make_unique<EntityScriptInstance>(entity.id(),
                                                         current_domain_,
                                                         type);

  instance->construct();

  return instance;
}

std::shared_ptr<EntityScriptType>
ScriptEngine::entity_script_type(const std::string &module_name)
{
  // search for entity script type
  const auto iter = entity_script_types_.find(module_name);
  if (iter == entity_script_types_.end())
  {
    // create entity script type
    if (module_name.empty())
    {
      DC_LOG_DEBUG("Can not construct script component with empty module");
      return {};
    }

    if (!is_valid_module(module_name))
    {
      DC_LOG_WARN("Can not find module named {}", module_name);
      return {};
    }

    const auto namespace_name = ScriptEngine::get_namespace_name(module_name);
    const auto class_name     = ScriptEngine::get_class_name(module_name);

    const auto type = std::make_shared<EntityScriptType>(module_name,
                                                         namespace_name,
                                                         class_name,
                                                         core_assembly_image_,
                                                         game_assembly_image_);
    entity_script_types_[module_name] = type;
    return type;
  }

  return iter->second;
}

} // namespace dc
