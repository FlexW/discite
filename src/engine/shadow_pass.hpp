#pragma once

#include "frame_data.hpp"
#include "gl_framebuffer.hpp"
#include "gl_shader.hpp"
#include "gl_texture_array.hpp"

#include <functional>

namespace dc
{

struct CascadeSplit
{
  float near{};
  float far{};
};

class ShadowPass
{
public:
  using Output =
      std::function<void(const SceneRenderInfo          &scene_render_info,
                         const ViewRenderInfo           &view_render_info,
                         std::shared_ptr<GlTextureArray> shadow_tex_array,
                         const std::vector<glm::mat4>    light_space_matrices,
                         const std::vector<CascadeSplit> cascade_frustums)>;

  ShadowPass();

  void execute(const SceneRenderInfo &scene_render_info,
               const ViewRenderInfo  &view_render_info);

  void set_output(Output output);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  Output output_;

  std::vector<MeshInfo> transparent_meshes_;
  std::vector<MeshInfo> solid_meshes_;

  int                       shadow_tex_width_{4096};
  int                       shadow_tex_height_{4096};
  unsigned                  shadow_cascades_count_{4};
  std::vector<CascadeSplit> cascade_frustums_;

  std::shared_ptr<GlTextureArray> shadow_tex_array_{};
  std::shared_ptr<GlFramebuffer>  shadow_framebuffer_{};

  std::shared_ptr<GlShader> shadow_map_shader_{};
  std::shared_ptr<GlShader> shadow_map_transparent_shader_{};

  std::shared_ptr<GlShader>      point_light_shadow_map_shader_{};
  std::shared_ptr<GlFramebuffer> point_light_framebuffer_{};

  void calc_shadow_cascades_splits(const ViewRenderInfo &view_render_info);

  std::vector<glm::mat4>
  calc_light_space_matrices(const DirectionalLight &directional_light,
                            const ViewRenderInfo   &view_render_info) const;

  glm::mat4 calc_light_space_matrix(const DirectionalLight &directional_light,
                                    const ViewRenderInfo   &view_render_info,
                                    float                   near_plane,
                                    float                   far_plane) const;

  std::vector<glm::vec4>
  calc_frustum_corners(const ViewRenderInfo &view_render_info,
                       const glm::mat4      &projection_matrix) const;

  void recreate_shadow_tex_framebuffer();

  void init_shaders();

  void generate_point_light_shadows(const SceneRenderInfo &scene_render_info);
};

} // namespace dc
