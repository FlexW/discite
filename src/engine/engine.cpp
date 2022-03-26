#include "engine.hpp"
#include "asset_importer_manager.hpp"
#include "cmd_args_parser.hpp"
#include "env_map_asset.hpp"
#include "event_manager.hpp"
#include "log.hpp"
#include "material_asset.hpp"
#include "mesh_asset.hpp"
#include "profiling.hpp"
#include "spdlog/logger.h"
#include "spdlog/sinks/ansicolor_sink.h"
#include "texture_asset.hpp"
#include "time.hpp"
#include "window.hpp"

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

Engine::Engine()
{
  event_manager_ = std::make_unique<EventManager>(
      [this](const Event &event) { layer_stack_.on_event(event); });
}

int Engine::run(int argc, char *argv[], bool show_window)
{
  try
  {
    DC_PROFILE_THREAD("main");
    DC_PROFILE_START_CAPTURE();

    init(argc, argv, show_window);

    DC_PROFILE_STOP_CAPTURE();
    DC_PROFILE_SAVE_CAPTURE("init");

    DC_PROFILE_START_CAPTURE();

    main_loop();

    DC_PROFILE_STOP_CAPTURE();
    DC_PROFILE_SAVE_CAPTURE("main_loop");

    DC_PROFILE_START_CAPTURE();

    shutdown();

    DC_PROFILE_STOP_CAPTURE();
    DC_PROFILE_SAVE_CAPTURE("shutdown");
  }
  catch (const std::runtime_error &error)
  {
    DC_PROFILE_SHUTDOWN();
    DC_LOG_ERROR("Unhandled exception: {}", error.what());
    return EXIT_FAILURE;
  }

  DC_PROFILE_SHUTDOWN();
  return EXIT_SUCCESS;
}

void Engine::push_layer(std::unique_ptr<Layer> layer)
{
  layer_stack_.push_layer(std::move(layer));
}

void Engine::load_config()
{
  const auto config_file_path = "data/engine.ini";
  try
  {
    config_->load_config(config_file_path);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not load config from {}", config_file_path);
  }
}

void Engine::init_logger()
{
  constexpr auto ansi_bold_red = "\033[1;31m";
  constexpr auto ansi_blue     = "\033[0;34m";
  constexpr auto ansi_yellow   = "\033[0;33m";
  constexpr auto ansi_grey     = "\033[0;90m";

  #ifdef _WIN32
  auto sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
  #else
  auto sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_st>();
  sink->set_color(spdlog::level::debug, ansi_grey);
  sink->set_color(spdlog::level::info, ansi_blue);
  sink->set_color(spdlog::level::warn, ansi_yellow);
  sink->set_color(spdlog::level::err, ansi_bold_red);
  #endif

  const auto logger = std::make_shared<spdlog::logger>("default", sink);
  spdlog::set_default_logger(logger);

  const auto log_pattern =
      config_->config_value_string("General", "log_pattern", "");
  if (!log_pattern.empty())
  {
    logger->set_pattern(log_pattern);
  }

  const auto debug_level =
      config_->config_value_string("General", "log_level", "debug");
  if (debug_level == "debug")
  {
    logger->set_level(spdlog::level::debug);
  }
  else if (debug_level == "info")
  {
    logger->set_level(spdlog::level::info);
  }
  else if (debug_level == "warn")
  {
    logger->set_level(spdlog::level::warn);
  }
  else if (debug_level == "error")
  {
    logger->set_level(spdlog::level::err);
  }
  else
  {
    DC_LOG_WARN("Unknown log level: {}", debug_level);
    logger->set_level(spdlog::level::info);
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
  init_logger();
  window_ = std::make_shared<Window>(show_window);
  layer_stack_.init();
}

void Engine::main_loop()
{
  auto last_time = current_time_millis();

  while (!window_->is_close() && !is_close_)
  {
    DC_PROFILE_FRAME("main");
    performance_profiler_.clear();
    {
      DC_TIME_SCOPE_PERF("Frame");

      {
        DC_PROFILE_SCOPE("Engine::main_loop() - Dispatch events");
        DC_TIME_SCOPE_PERF("Dispatch events");
        window_->dispatch_events();
        event_manager_->dispatch_events();
      }

      // calculate delta time
      const auto delta_time = (current_time_millis() - last_time) / 1000.0f;
      last_time             = current_time_millis();

      {
        DC_PROFILE_SCOPE("Engine::main_loop() - Layers update");
        DC_TIME_SCOPE_PERF("Layers update");
        layer_stack_.update(delta_time);
      }
      {
        DC_PROFILE_SCOPE("Engine::main_loop() - Layers render");
        DC_TIME_SCOPE_PERF("Layers render");
        layer_stack_.render();
      }

      {
        DC_PROFILE_SCOPE("Engine::main_loop() - Swap buffers");
        DC_TIME_SCOPE_PERF("Swap buffers");
        window_->swap_buffers();
      }
    }
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

PerformanceProfiler *Engine::performance_profiler()
{
  return &performance_profiler_;
}

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
