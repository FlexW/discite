#pragma once

#include "frame_data.hpp"
#include "gl_framebuffer.hpp"
#include "gl_shader.hpp"

#include <functional>

namespace dc
{

class SkyboxPass
{
public:
  using Output =
      std::function<void(const SceneRenderInfo         &scene_render_info,
                         const ViewRenderInfo          &view_render_info,
                         std::shared_ptr<GlFramebuffer> scene_framebuffer)>;

  SkyboxPass();
  ~SkyboxPass();

  void execute(const SceneRenderInfo         &scene_render_info,
               const ViewRenderInfo          &view_render_info,
               std::shared_ptr<GlFramebuffer> scene_framebuffer);

  void set_output(Output output);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  Output output_;

  std::shared_ptr<GlShader> sky_box_shader_{};
  bool                      is_show_irradiance_as_skybox_{true};

  GLuint cube_vertex_array_{0};
  GLuint cube_vertex_buffer_{0};

  void init_shaders();
  void init_cube();
  void render_cube();
};

} // namespace dc
