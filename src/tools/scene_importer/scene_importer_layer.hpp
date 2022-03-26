#pragma once

#include "layer.hpp"

namespace dc
{

class SceneImporterLayer : public Layer
{
public:
  void add_cmd_line_args(ArgsParser &args_parser) override;
  void eval_cmd_line_args(ArgsParser &args_parser) override;

  void init() override;
  void shutdown() override;

  bool update(float delta_time) override;
  bool render() override;

  bool on_event(const Event &event) override;

private:
  std::string file_path_;
  std::string name_;
};

} // namespace dc
