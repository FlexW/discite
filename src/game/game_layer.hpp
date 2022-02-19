#pragma once

#include "layer.hpp"
#include "renderer.hpp"
#include "scene.hpp"

#include <memory>

class GameLayer : public Layer
{
public:
  void init() override;
  void shutdown() override;
  void update(float delta_time) override;
  void render() override;

  bool on_event(const Event &event) override;

  std::shared_ptr<Renderer> renderer() const;

private:
  std::shared_ptr<Scene>    scene_;
  std::shared_ptr<Renderer> renderer_;
};
