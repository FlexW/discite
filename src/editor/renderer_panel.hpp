#pragma once

#include "imgui_panel.hpp"
#include "renderer.hpp"

#include <memory>

namespace dc
{

class RendererPanel : public ImGuiPanel
{
public:
  RendererPanel();

  void set_renderer(std::shared_ptr<Renderer> renderer);

private:
  std::weak_ptr<Renderer> renderer_{};

  void on_render() override;

  void render_general();
  void render_shadows();
};

} // namespace dc
