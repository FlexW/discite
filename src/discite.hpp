#pragma once

#include "engine/game.hpp"
#include "renderer.hpp"
#include "scene.hpp"

#include <memory>

class Discite : public Game
{
public:
  void init() override;
  void update(float delta_time) override;
  void render() override;
  void render_imgui() override;

private:
  std::shared_ptr<Scene>    scene_;
  std::unique_ptr<Renderer> renderer_;
};
