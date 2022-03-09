#include "layer_stack.hpp"

namespace dc
{

void LayerStack::push_layer(std::unique_ptr<Layer> layer)
{
  layers_.push_back(std::move(layer));
}

void LayerStack::add_cmd_line_args(ArgsParser &args_parser)
{
  for (auto &layer : layers_)
  {
    layer->add_cmd_line_args(args_parser);
  }
}

void LayerStack::eval_cmd_line_args(ArgsParser &args_parser)
{
  for (auto &layer : layers_)
  {
    layer->eval_cmd_line_args(args_parser);
  }
}

void LayerStack::register_asset_loaders()
{
  for (const auto &layer : layers_)
  {
    layer->register_asset_loaders();
  }
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

} // namespace dc
