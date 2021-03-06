#pragma once

#include "layer.hpp"

namespace dc
{

class MeshImporterLayer : public Layer
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
  std::string mesh_name_;
  bool        is_skinned_{false};
};

} // namespace dc
