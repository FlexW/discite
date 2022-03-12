#pragma once

#include "frame_data.hpp"
#include "gl_framebuffer.hpp"
#include "gl_shader.hpp"
#include "gl_texture.hpp"
#include "render_pass.hpp"
#include "shadow_pass.hpp"

namespace dc
{

class ForwardPass
{
public:
  using Output =
      std::function<void(const SceneRenderInfo         &scene_render_info,
                         const ViewRenderInfo          &view_render_info,
                         std::shared_ptr<GlFramebuffer> scene_framebuffer)>;

  ForwardPass();

  void execute(const SceneRenderInfo          &scene_render_info,
               const ViewRenderInfo           &view_render_info,
               std::shared_ptr<GlTextureArray> shadow_tex_array,
               const std::vector<glm::mat4>    light_space_matrices,
               const std::vector<CascadeSplit> cascade_frustums);

  void set_output(Output output);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  Output output_;

  int                            scene_framebuffer_width_{0};
  int                            scene_framebuffer_height_{0};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_{};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_msaa_{};

  std::shared_ptr<GlTexture> white_texture_{};

  std::shared_ptr<GlShader> mesh_shader_{};

  std::shared_ptr<GlTexture> brdf_lut_texture_{};

  float light_size_{2.0f};
  float shadow_bias_min_{0.0f};
  bool  smooth_shadows_{true};
  bool  show_shadow_cascades_{false};
  bool  is_shadows_enabled_{true};

  void init_shaders();
  void recreate_scene_framebuffer(int width, int height);
};

} // namespace dc
