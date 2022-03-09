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
  ~RendererPanel();

  void set_renderer(std::shared_ptr<Renderer> renderer);

private:
  std::weak_ptr<Renderer> renderer_{};

  GLuint                         quad_vertex_array_{0};
  std::shared_ptr<GlShader>      depth_debug_shader_{};
  std::shared_ptr<GlFramebuffer> debug_quad_framebuffer_{};
  int                            debug_quad_height_{0};
  int                            debug_quad_width_{0};
  std::size_t                    debug_selected_cascade_{0};

  void on_render() override;

  void render_general();
  void render_shadows();

  void recreate_debug_quad_framebuffer(int new_width, int new_height);
};

} // namespace dc
