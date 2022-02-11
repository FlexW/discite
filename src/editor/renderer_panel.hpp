#pragma once

#include "imgui_panel.hpp"
#include "renderer.hpp"

#include <memory>

class RendererPanel : public ImGuiPanel
{
public:
  void render() override;

  void set_renderer(std::shared_ptr<Renderer> renderer);

private:
  std::weak_ptr<Renderer> renderer_{};

  void render_general();
  void render_shadows();
};
