#pragma once

#include "frame_data.hpp"
#include "graphic/renderer.hpp"
#include "render_pass.hpp"
#include "shadow_pass.hpp"

#include <memory>

namespace dc
{

class ForwardPass
{
public:
  using Output =
      std::function<void(const SceneRenderInfo       &scene_render_info,
                         const ViewRenderInfo        &view_render_info,
                         std::shared_ptr<Framebuffer> scene_framebuffer,
                         std::shared_ptr<CubeTexture> sky_irradiance_map)>;

  ForwardPass(std::shared_ptr<Renderer> renderer);

  void execute(const SceneRenderInfo           &scene_render_info,
               const ViewRenderInfo            &view_render_info,
               std::shared_ptr<TextureArray>    shadow_tex_array,
               const std::vector<glm::mat4>    &light_space_matrices,
               const std::vector<CascadeSplit> &cascade_frustums);

  void set_output(Output output);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  struct EnvMapData
  {
    std::shared_ptr<CubeTexture> irradiance_tex_{};
    std::shared_ptr<CubeTexture> prefilter_tex_{};
  };

  std::shared_ptr<Renderer> renderer_{};

  std::unordered_map<std::string, EnvMapData> env_maps_;

  bool                      need_generate_prefilter_map_{true};
  std::shared_ptr<GlShader> equirectangular_to_cubemap_shader_{};
  std::shared_ptr<GlShader> prefilter_shader_{};
  std::shared_ptr<GlShader> irradiance_shader_{};

  Output output_;

  int                            scene_framebuffer_width_{0};
  int                            scene_framebuffer_height_{0};
  std::shared_ptr<Framebuffer>   scene_framebuffer_{};
  std::shared_ptr<Framebuffer>   scene_framebuffer_msaa_{};

  std::shared_ptr<Texture>     white_texture_{};
  std::shared_ptr<CubeTexture> dummy_cube_texture_{};

  std::shared_ptr<Shader> line_shader_{};
  std::shared_ptr<Shader> depth_only_shader_{};
  std::shared_ptr<Shader> skinned_depth_only_shader_{};
  std::shared_ptr<Shader> mesh_shader_{};
  std::shared_ptr<Shader> skinned_mesh_shader_{};

  static constexpr std::size_t    max_debug_lines_count{1024 * 8};
  std::shared_ptr<VertexBuffer>   lines_vertex_buffer_{};
  // std::shared_ptr<GlVertexArray>  lines_vertex_array_;

  std::shared_ptr<Texture> brdf_lut_texture_{};

  float light_size_{2.0f};
  float shadow_bias_min_{0.0f};
  bool  smooth_shadows_{true};
  bool  show_shadow_cascades_{false};

  void init_shaders();
  void recreate_scene_framebuffer(int width, int height);

  EnvMapData env_map(const EnvironmentMap &env_map);
  EnvMapData generate_env_map(const EnvironmentMap &env_map);

  void render_meshes_depth_prepass(const SceneRenderInfo &scene_render_info,
                                   const ViewRenderInfo  &view_render_info);

  void
  render_skinned_meshes_depth_prepass(const SceneRenderInfo &scene_render_info,
                                      const ViewRenderInfo  &view_render_info);

  void set_lightning(GlShader                        &shader,
                     int                             &global_texture_slot,
                     const SceneRenderInfo           &scene_render_info,
                     const ViewRenderInfo            &view_render_info,
                     std::shared_ptr<GlTextureArray>  shadow_tex_array,
                     const std::vector<glm::mat4>    &light_space_matrices,
                     const std::vector<CascadeSplit> &cascade_frustums);

  void set_ibl(GlShader         &shader,
               int              &global_texture_slot,
               const EnvMapData &env_map_data);

  void render_meshes(int                             &global_texture_slot,
                     const EnvMapData                &env_map_data,
                     const SceneRenderInfo           &scene_render_info,
                     const ViewRenderInfo            &view_render_info,
                     std::shared_ptr<GlTextureArray>  shadow_tex_array,
                     const std::vector<glm::mat4>    &light_space_matrices,
                     const std::vector<CascadeSplit> &cascade_frustums);

  void render_skinned_meshes(int                   &global_texture_slot,
                             const EnvMapData      &env_map_data,
                             const SceneRenderInfo &scene_render_info,
                             const ViewRenderInfo  &view_render_info,
                             std::shared_ptr<GlTextureArray> shadow_tex_array,
                             const std::vector<glm::mat4> &light_space_matrices,
                             const std::vector<CascadeSplit> &cascade_frustums);

  void render_debug_lines(const SceneRenderInfo &scene_render_info,
                          const ViewRenderInfo  &view_render_info);

  void
  set_material(GlShader &shader, int &texture_slot, const Material &material);
};

} // namespace dc
