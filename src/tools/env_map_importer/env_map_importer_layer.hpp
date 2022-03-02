#pragma once

#include "layer.hpp"

#include <array>
#include <filesystem>

class EnvMapImporter : public Layer
{
public:
  void add_cmd_line_args(ArgsParser &args_parser) override;
  void eval_cmd_line_args(ArgsParser &args_parser) override;

  void init() override;
  void shutdown() override;
  void update(float delta_time) override;
  void render() override;

  bool on_event(const Event &event) override;

private:
  std::string           name_;
  std::filesystem::path file_path_;
};
