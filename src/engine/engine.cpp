#include "engine.hpp"
#include "log.hpp"
#include "time.hpp"
#include "window.hpp"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Engine *Engine::instance()
{
  static auto unique = std::unique_ptr<Engine>(new Engine);
  return unique.get();
}

int Engine::run()
{
  try
  {
    init();
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

void Engine::set_game(std::unique_ptr<Game> game) { game_ = std::move(game); }

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

void Engine::init()
{
  load_config();
  set_log_level();
  window_ = std::make_unique<Window>();
  init_imgui();
  assert(game_);
  game_->init();
}

void Engine::main_loop()
{
  assert(game_);

  auto last_time = current_time_millis();

  while (!window_->is_close() && !is_close_)
  {
    window_->dispatch_events();
    event_manager_->dispatch();

    // calculate delta time
    const auto delta_time = (current_time_millis() - last_time) / 1000.0f;
    last_time             = current_time_millis();

    game_->update(delta_time);
    game_->render();

    // render imgui
    {
      // start imgui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      game_->render_imgui();
      // finish imgui frame
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    window_->swap_buffers();
  }
}

void Engine::shutdown()
{
  // make sure game gets unloaded before the engine stops
  game_ = nullptr;

  // stop the window
  shutdown_imgui();
  window_ = nullptr;

  // shutdown other systems
  event_manager_ = nullptr;
  config_        = nullptr;
}

void Engine::set_close(bool value) { is_close_ = value; }

void Engine::init_imgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window_->handle(), true);
  ImGui_ImplOpenGL3_Init("#version 460 core");
}

void Engine::shutdown_imgui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

Config *Engine::config() const { return config_.get(); }

Window *Engine::window() const { return window_.get(); }

EventManager *Engine::event_manager() const { return event_manager_.get(); }
