#include "mesh_importer_layer.hpp"
#include "cmd_args_parser.hpp"
#include "engine.hpp"
#include "mesh_asset_importer.hpp"
#include "skinned_mesh_asset_importer.hpp"

#include <filesystem>
#include <stdexcept>

namespace dc
{

void MeshImporterLayer::add_cmd_line_args(ArgsParser &args_parser)
{
  ArgsParser::Option file_path_option;
  file_path_option.name_        = "file-path";
  file_path_option.description_ = "File path of mesh to import";
  file_path_option.type_        = ArgsParser::OptionType::Value;
  file_path_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(file_path_option);

  ArgsParser::Option skinned_option;
  skinned_option.name_        = "skinned";
  skinned_option.description_ = "Import the mesh with skinning info";
  skinned_option.type_        = ArgsParser::OptionType::NonValue;
  skinned_option.importance_  = ArgsParser::OptionImportance::Optional;
  args_parser.add_option(skinned_option);

  ArgsParser::Option name_option;
  name_option.name_        = "name";
  name_option.description_ = "Name of the imported mesh";
  name_option.type_        = ArgsParser::OptionType::Value;
  name_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(name_option);
}

void MeshImporterLayer::eval_cmd_line_args(ArgsParser &args_parser)
{
  file_path_ = args_parser.value_as_string("file-path").value();
  mesh_name_  = args_parser.value_as_string("name").value();
  is_skinned_ = args_parser.is_option_set("skinned");
}

void MeshImporterLayer::init()
{
  try
  {
    if (is_skinned_)
    {
      import_skinned_mesh_asset(std::filesystem::path{file_path_}, mesh_name_);
    }
    else
    {
      import_mesh_asset(std::filesystem::path{file_path_}, mesh_name_);
    }
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_ERROR("Error while importing mesh {}", error.what());
  }
}

void MeshImporterLayer::shutdown() {}

bool MeshImporterLayer::update(float /*delta_time*/)
{
  Engine::instance()->set_close(true);

  return false;
}

bool MeshImporterLayer::render() { return false; }

bool MeshImporterLayer::on_event(const Event & /*event*/) { return false; }

} // namespace dc
