#pragma once

#include "frame_data.hpp"
#include "gl_framebuffer.hpp"
#include "gl_shader.hpp"

#include <functional>

namespace dc
{

class HdrPass
{
public:
  using Output =
      std::function<void(const SceneRenderInfo &,
                         const ViewRenderInfo &,
                         std::shared_ptr<GlFramebuffer> scene_framebuffer)>;

  HdrPass();
  ~HdrPass();

  void execute(const SceneRenderInfo         &scene_render_info,
               const ViewRenderInfo          &view_render_info,
               std::shared_ptr<GlFramebuffer> scene_framebuffer);

  void set_output(Output output);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  Output output_;

  float                     exposure_{0.01f};
  std::shared_ptr<GlShader> hdr_shader_{};

  GLuint quad_vertex_array_{0};

  void init_shaders();
};

} // namespace dc
