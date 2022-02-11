#pragma once

#include "layer.hpp"

#include <cassert>
#include <memory>
#include <vector>

class LayerStack
{
public:
  virtual ~LayerStack() = default;

  void push_layer(std::unique_ptr<Layer> layer);

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
    assert(0);
    return nullptr;
  }

private:
  std::vector<std::unique_ptr<Layer>> layers_;
};
