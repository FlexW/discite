#include "engine.hpp"
#include "log.hpp"
#include "time.hpp"
#include "window.hpp"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <stdexcept>

Engine *Engine::instance()
{
  static auto unique = std::unique_ptr<Engine>(new Engine);
  return unique.get();
}

int Engine::run(bool show_window)
{
  try
  {
    init(show_window);
    main_loop();
    shutdown();
  }
  catch (const std::runtime_error &error)
  {
    LOG_ERROR() << "Unhandled exception: " << error.what();
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
    LOG_WARN() << "Could not load config from " << config_file_path.string();
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
    LOG_WARN() << "Unknown log level: " << debug_level;
    Log::set_reporting_level(LogLevel::Debug);
  }
}

void Engine::init(bool show_window)
{
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

    // // render imgui
    // {
    //   // start imgui frame
    //   ImGui_ImplOpenGL3_NewFrame();
    //   ImGui_ImplGlfw_NewFrame();
    //   ImGui::NewFrame();
    //   // finish imgui frame
    //   ImGui::Render();
    //   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // }

    window_->swap_buffers();
  }
}

void Engine::shutdown()
{
  // make sure layers get unloaded before the engine stops
  layer_stack_.shutdown();

  // stop the window
  // shutdown_imgui();
  window_ = nullptr;

  // shutdown other systems
  event_manager_ = nullptr;
  config_        = nullptr;
}

void Engine::set_close(bool value) { is_close_ = value; }

void Engine::init_imgui()
{
  // IMGUI_CHECKVERSION();
  // ImGui::CreateContext();
  // ImGui::StyleColorsDark();

  // ImGui_ImplGlfw_InitForOpenGL(window_->handle(), true);
  // ImGui_ImplOpenGL3_Init("#version 460 core");
}

void Engine::shutdown_imgui()
{
  // ImGui_ImplOpenGL3_Shutdown();
  // ImGui_ImplGlfw_Shutdown();
  // ImGui::DestroyContext();
}

Config *Engine::config() const { return config_.get(); }

Window *Engine::window() const { return window_.get(); }

EventManager *Engine::event_manager() const { return event_manager_.get(); }

LayerStack *Engine::layer_stack() { return &layer_stack_; }
