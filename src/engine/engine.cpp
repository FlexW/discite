#include "engine.hpp"
#include "asset_importer_manager.hpp"
#include "cmd_args_parser.hpp"
#include "env_map_asset.hpp"
#include "log.hpp"
#include "material_asset.hpp"
#include "mesh_asset.hpp"
#include "texture_asset.hpp"
#include "time.hpp"
#include "window.hpp"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <stdexcept>

namespace dc
{

Engine *Engine::instance()
{
  static auto unique = std::unique_ptr<Engine>(new Engine);
  return unique.get();
}

int Engine::run(int argc, char *argv[], bool show_window)
{
  try
  {
    init(argc, argv, show_window);
    main_loop();
    shutdown();
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_ERROR() << "Unhandled exception: " << error.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void Engine::push_layer(std::unique_ptr<Layer> layer)
{
  layer_stack_.push_layer(std::move(layer));
}

void Engine::load_config()
{
  const auto config_file_path = project_path_ / "engine.ini";
  try
  {
    config_->load_config(config_file_path);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN() << "Could not load config from " << config_file_path.string();
  }
}

void Engine::set_log_level()
{

  const auto debug_level =
      config_->config_value_string("General", "log_level", "debug");
  if (debug_level == "debug")
  {
    Log::set_reporting_level(LogLevel::Debug);
  }
  else if (debug_level == "info")
  {
    Log::set_reporting_level(LogLevel::Info);
  }
  else if (debug_level == "warn")
  {
    Log::set_reporting_level(LogLevel::Warning);
  }
  else if (debug_level == "error")
  {
    Log::set_reporting_level(LogLevel::Error);
  }
  else
  {
    DC_LOG_WARN() << "Unknown log level: " << debug_level;
    Log::set_reporting_level(LogLevel::Debug);
  }
}

void Engine::init(int argc, char *argv[], bool show_window)
{
  ArgsParser args_parser;

  ArgsParser::Option data_directory_option;
  data_directory_option.name_        = "data-directory";
  data_directory_option.description_ = "Absolute path to game data directory";
  data_directory_option.type_        = ArgsParser::OptionType::Value;
  data_directory_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(data_directory_option);

  // process command line args
  layer_stack_.add_cmd_line_args(args_parser);
  parse_and_show_help_on_error(args_parser, argc, argv);
  layer_stack_.eval_cmd_line_args(args_parser);

  base_directory_ =
      args_parser.value_as_string(data_directory_option.name_).value();
  if (!base_directory_.is_absolute())
  {
    throw std::runtime_error("Data directory needs to be absolute");
  }

  register_asset_loaders();
  layer_stack_.register_asset_loaders();
  load_config();
  set_log_level();
  window_ = std::make_shared<Window>(show_window);
  layer_stack_.init();
}

void Engine::main_loop()
{
  auto last_time = current_time_millis();

  while (!window_->is_close() && !is_close_)
  {
    window_->dispatch_events();
    event_manager_->dispatch([this](const Event &event)
                             { layer_stack_.on_event(event); });

    // calculate delta time
    const auto delta_time = (current_time_millis() - last_time) / 1000.0f;
    last_time             = current_time_millis();

    layer_stack_.update(delta_time);
    layer_stack_.render();

    window_->swap_buffers();
  }
}

void Engine::shutdown()
{
  // make sure layers get unloaded before the engine stops
  layer_stack_.shutdown();

  // stop the window
  window_ = nullptr;

  // shutdown other systems
  event_manager_ = nullptr;
  config_        = nullptr;
}

void Engine::set_close(bool value) { is_close_ = value; }

Config *Engine::config() const { return config_.get(); }

Window *Engine::window() const { return window_.get(); }

EventManager *Engine::event_manager() const { return event_manager_.get(); }

LayerStack *Engine::layer_stack() { return &layer_stack_; }

AssetCache *Engine::asset_cache() const { return asset_cache_.get(); }

AssetImporterManager *Engine::asset_importer_manager() const
{
  return asset_importer_manager_.get();
}

std::filesystem::path Engine::base_directory() const { return base_directory_; }

void Engine::register_asset_loaders()
{
  asset_cache_->register_asset_loader(".dctex", texture_asset_loader);
  asset_cache_->register_asset_loader(".dcmat", material_asset_loader);
  asset_cache_->register_asset_loader(".dcmesh", mesh_asset_loader);
  asset_cache_->register_asset_loader(".dcenv", env_map_asset_loader);
}

} // namespace dc
