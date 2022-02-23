#pragma once

#include "config.hpp"
#include "event_manager.hpp"
#include "gl.hpp"
#include "layer_stack.hpp"
#include "log.hpp"
#include "window.hpp"

#include <filesystem>
#include <memory>

class Engine
{
public:
  static Engine *instance();

  int run(bool show_window = true);

  void push_layer(std::unique_ptr<Layer> layer);

  void set_close(bool value);

  Config       *config() const;
  Window       *window() const;
  EventManager *event_manager() const;
  LayerStack   *layer_stack();

private:
  std::filesystem::path project_path_ = std::filesystem::current_path();

  std::unique_ptr<Config>       config_{std::make_unique<Config>()};
  LayerStack                    layer_stack_;
  std::shared_ptr<Window>       window_;
  std::unique_ptr<EventManager> event_manager_{
      std::make_unique<EventManager>()};

  bool is_close_{false};

  Engine()               = default;
  Engine(const Engine &) = delete;
  Engine(Engine &&)      = delete;
  Engine &operator=(const Engine &) = delete;
  Engine &operator=(Engine &&) = delete;

  void init(bool show_window);
  void main_loop();
  void shutdown();
  void init_imgui();
  void shutdown_imgui();

  void load_config();
  void set_log_level();
};
