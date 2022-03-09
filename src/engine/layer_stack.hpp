#pragma once

#include "assert.hpp"
#include "cmd_args_parser.hpp"
#include "layer.hpp"

#include <memory>
#include <vector>

namespace dc
{

class LayerStack
{
public:
  virtual ~LayerStack() = default;

  void push_layer(std::unique_ptr<Layer> layer);

  void add_cmd_line_args(ArgsParser &args_parser);
  void eval_cmd_line_args(ArgsParser &args_parser);

  void register_asset_loaders();

  void init();
  void shutdown();
  void update(float delta_time);
  void render();

  bool on_event(const Event &event);

  template <typename TLayer> TLayer *layer() const
  {
    for (const auto &layer : layers_)
    {
      // it's fine to dynamic cast since we don't expect much layers
      const auto l = dynamic_cast<TLayer *>(layer.get());
      if (l)
      {
        return l;
      }
    }
    // that's for sure a programming mistake
    DC_FAIL("No layer of that type");
    return nullptr;
  }

private:
  std::vector<std::unique_ptr<Layer>> layers_;
};

} // namespace dc
