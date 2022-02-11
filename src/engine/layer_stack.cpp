#include "layer_stack.hpp"

void LayerStack::push_layer(std::unique_ptr<Layer> layer)
{
  layers_.push_back(std::move(layer));
}

void LayerStack::init()
{
  for (const auto &layer : layers_)
  {
    layer->init();
  }
}

void LayerStack::shutdown() { layers_.clear(); }

void LayerStack::update(float delta_time)
{
  for (const auto &layer : layers_)
  {
    layer->update(delta_time);
  }
}

void LayerStack::render()
{
  for (const auto &layer : layers_)
  {
    layer->render();
  }
}

bool LayerStack::on_event(const Event &event)
{
  for (const auto &layer : layers_)
  {
    if (layer->on_event(event))
    {
      return true;
    }
  }
  return false;
}
