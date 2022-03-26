#include "env_map_importer_layer.hpp"
#include "asset.hpp"
#include "cmd_args_parser.hpp"
#include "defer.hpp"
#include "engine.hpp"
#include "gl_shader.hpp"
#include "gl_shader_storage_buffer.hpp"
#include "importer.hpp"
#include "log.hpp"
#include "math.hpp"
#include "serialization.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <stdexcept>

namespace dc
{

void EnvMapImporter::add_cmd_line_args(ArgsParser &args_parser)
{
  ArgsParser::Option hdr_map_option;
  hdr_map_option.name_        = "file-path";
  hdr_map_option.description_ = "File path of HDR map to import";
  hdr_map_option.type_        = ArgsParser::OptionType::Value;
  hdr_map_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(hdr_map_option);

  ArgsParser::Option name_option;
  name_option.name_        = "name";
  name_option.description_ = "Name of the imported environment map";
  name_option.type_        = ArgsParser::OptionType::Value;
  name_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(name_option);
}

void EnvMapImporter::eval_cmd_line_args(ArgsParser &args_parser)
{

  file_path_ = args_parser.value_as_string("file-path").value();
  name_      = args_parser.value_as_string("name").value();
}

void EnvMapImporter::init() {}

void EnvMapImporter::shutdown() {}

bool EnvMapImporter::update(float /*delta_time*/)
{
  const auto base_directory = Engine::instance()->base_directory();

  {
    const auto file = std::fopen(file_path_.string().c_str(), "rb");
    if (!file)
    {
      throw std::runtime_error("Could not open file " + file_path_.string());
    }
    defer(std::fclose(file));

    std::fseek(file, 0, SEEK_END);
    const auto size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    std::vector<std::uint8_t> hdr_map_data(size);
    std::fread(hdr_map_data.data(), sizeof(std::uint8_t), size, file);

    const auto base_directory = Engine::instance()->base_directory();
    std::filesystem::create_directories(base_directory / "envs");

    AssetDescription          asset_description{};
    EnvironmentMapDescription env_map_description{};
    env_map_description.env_map_data_     = std::move(hdr_map_data);
    env_map_description.save(base_directory / "envs" /
                                 sanitize_file_path(name_ + ".dcenv"),
                             asset_description);
  }

  Engine::instance()->set_close(true);

  return false;
}

bool EnvMapImporter::render() { return false; }

bool EnvMapImporter::on_event(const Event & /*event*/) { return false; }

} // namespace dc
