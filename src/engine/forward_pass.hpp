#pragma once

#include "frame_data.hpp"
#include "gl_cube_texture.hpp"
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
      std::function<void(const SceneRenderInfo &        scene_render_info,
                         const ViewRenderInfo &         view_render_info,
                         std::shared_ptr<GlFramebuffer> scene_framebuffer,
                         std::shared_ptr<GlCubeTexture> sky_irradiance_map)>;

  ForwardPass();

  void execute(const SceneRenderInfo &         scene_render_info,
               const ViewRenderInfo &          view_render_info,
               std::shared_ptr<GlTextureArray> shadow_tex_array,
               const std::vector<glm::mat4>    light_space_matrices,
               const std::vector<CascadeSplit> cascade_frustums);

  void set_output(Output output);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  struct EnvMapData
  {
    std::shared_ptr<GlCubeTexture> irradiance_tex_{};
    std::shared_ptr<GlCubeTexture> prefilter_tex_{};
  };

  std::unordered_map<std::string, EnvMapData> env_maps_;

  bool                      need_generate_prefilter_map_{true};
  std::shared_ptr<GlShader> equirectangular_to_cubemap_shader_{};
  std::shared_ptr<GlShader> prefilter_shader_{};
  std::shared_ptr<GlShader> irradiance_shader_{};

  Output output_;

  int                            scene_framebuffer_width_{0};
  int                            scene_framebuffer_height_{0};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_{};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_msaa_{};

  std::shared_ptr<GlTexture> white_texture_{};

  std::shared_ptr<GlShader> depth_only_shader_;
  std::shared_ptr<GlShader> mesh_shader_{};

  std::shared_ptr<GlTexture> brdf_lut_texture_{};

  float light_size_{2.0f};
  float shadow_bias_min_{0.0f};
  bool  smooth_shadows_{true};
  bool  show_shadow_cascades_{false};
  bool  is_shadows_enabled_{true};

  void init_shaders();
  void recreate_scene_framebuffer(int width, int height);

  EnvMapData env_map(EnvironmentMap &env_map);
  EnvMapData generate_env_map(const EnvironmentMap &env_map);
};

} // namespace dc
