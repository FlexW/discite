#pragma once

#include "cmd_args_parser.hpp"
#include "event.hpp"

namespace dc
{

class Layer
{
public:
  virtual ~Layer() = default;

  virtual void add_cmd_line_args(ArgsParser & /*args_parser*/) {}
  virtual void eval_cmd_line_args(ArgsParser & /*args_parser*/) {}

  virtual void register_asset_loaders() {}

  virtual void init()                   = 0;
  virtual void shutdown()               = 0;
  virtual void update(float delta_time) = 0;
  virtual void render()                 = 0;

  virtual bool on_event(const Event &event) = 0;
};

} // namespace dc
