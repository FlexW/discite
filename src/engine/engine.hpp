#pragma once

#include "asset_cache.hpp"
#include "asset_importer_manager.hpp"
#include "config.hpp"
#include "event_manager.hpp"
#include "gl.hpp"
#include "layer_stack.hpp"
#include "log.hpp"
#include "window.hpp"

#include <filesystem>
#include <memory>

namespace dc
{

class Engine
{
public:
  static Engine *instance();

  int run(int argc, char *argv[], bool show_window = true);

  void push_layer(std::unique_ptr<Layer> layer);

  void set_close(bool value);

  Config       *config() const;
  Window       *window() const;
  EventManager *event_manager() const;
  LayerStack   *layer_stack();

  AssetCache           *asset_cache() const;
  AssetImporterManager *asset_importer_manager() const;
  std::filesystem::path base_directory() const;

private:
  std::filesystem::path project_path_ = std::filesystem::current_path();

  std::unique_ptr<Config>       config_{std::make_unique<Config>()};
  LayerStack                    layer_stack_;
  std::shared_ptr<Window>       window_;
  std::unique_ptr<EventManager> event_manager_{
      std::make_unique<EventManager>()};
  std::unique_ptr<AssetCache> asset_cache_{std::make_unique<AssetCache>()};
  std::unique_ptr<AssetImporterManager> asset_importer_manager_{
      std::make_unique<AssetImporterManager>()};

  std::filesystem::path base_directory_{"data"};

  bool is_close_{false};

  Engine()               = default;
  Engine(const Engine &) = delete;
  Engine(Engine &&)      = delete;
  Engine &operator=(const Engine &) = delete;
  Engine &operator=(Engine &&) = delete;

  void init(int argc, char *argv[], bool show_window);
  void main_loop();
  void shutdown();

  void load_config();
  void init_logger();

  void register_asset_loaders();
};

} // namespace dc
