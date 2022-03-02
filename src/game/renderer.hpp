#pragma once

#include "directional_light.hpp"
#include "environment_map.hpp"
#include "gl_cube_texture.hpp"
#include "gl_framebuffer.hpp"
#include "gl_shader.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "point_light.hpp"
#include "sky.hpp"

#include <memory>

struct MeshInfo
{
  glm::mat4 model_matrix_;
  Mesh     *mesh_;
};

class SceneRenderInfo
{
public:
  void                  add_mesh(const MeshInfo &mesh_info);
  std::vector<MeshInfo> meshes() const;

  void                    add_point_light(const PointLight &point_light);
  std::vector<PointLight> point_lights() const;

  void set_directional_light(const DirectionalLight &directional_light);
  DirectionalLight directional_light() const;

  void          set_env_map(const EnvionmentMap &sky);
  EnvionmentMap env_map() const;

private:
  std::vector<MeshInfo>   meshes_;
  std::vector<PointLight> point_lights_;
  DirectionalLight        directional_light_;
  EnvionmentMap           env_map_;
};

struct ViewportInfo
{
  int x_{};
  int y_{};
  int width_{};
  int height_{};
};

class ViewRenderInfo
{
public:
  void      set_view_matrix(const glm::mat4 &value);
  glm::mat4 view_matrix() const;

  void      set_projection_matrix(const glm::mat4 &value);
  glm::mat4 projection_matrix() const;

  void         set_viewport_info(const ViewportInfo &value);
  ViewportInfo viewport_info() const;

  void  set_aspect_ratio(float value);
  float aspect_ratio() const;

  void  set_near_plane(float value);
  float near_plane() const;

  void  set_far_plane(float value);
  float far_plane() const;

  void  set_fov(float value);
  float fov() const;

private:
  float        fov_{};
  float        aspect_ratio_{};
  float        near_plane_{};
  float        far_plane_{};
  ViewportInfo viewport_info_{};
  glm::mat4    view_matrix_{1.0f};
  glm::mat4    projection_matrix_{1.0f};
};

class Renderer
{
public:
  Renderer();
  ~Renderer();

  void render(const SceneRenderInfo         &scene_render_info,
              const ViewRenderInfo          &view_render_info,
              std::optional<GlFramebuffer *> framebuffer = {});

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  struct CascadeSplit
  {
    float near{};
    float far{};
  };

  std::shared_ptr<GlShader> mesh_shader_;

  std::shared_ptr<GlTexture> white_texture_{};

  std::shared_ptr<GlShader>       shadow_map_shader_{};
  std::shared_ptr<GlShader>       shadow_map_transparent_shader_{};
  std::shared_ptr<GlTextureArray> shadow_tex_array_{};
  bool                            is_shadows_enabled_{true};
  bool                            show_shadow_cascades_{false};
  float                           light_size_{10.25f};
  float                           shadow_bias_min_{0.0f};
  bool                            smooth_shadows_{true};
  int                             shadow_tex_width_{4096};
  int                             shadow_tex_height_{4096};
  unsigned                        shadow_cascades_count_{4};
  std::vector<CascadeSplit>       cascade_frustums_;

  std::shared_ptr<GlFramebuffer> shadow_framebuffer_{};

  GLuint                         quad_vertex_array_{};
  std::shared_ptr<GlShader>      depth_debug_shader_{};
  std::shared_ptr<GlFramebuffer> debug_quad_framebuffer_{};
  int                            debug_quad_height_{0};
  int                            debug_quad_width_{0};
  std::size_t                    debug_selected_cascade_{0};

  int                            scene_framebuffer_width_{0};
  int                            scene_framebuffer_height_{0};
  float                          exposure_{0.01f};
  std::shared_ptr<GlShader>      hdr_shader_{};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_{};

  std::vector<MeshInfo> transparent_meshes_;
  std::vector<MeshInfo> solid_meshes_;

  std::shared_ptr<GlTexture> brdf_lut_texture_{};

  void load_shaders();

  void recalculate_projection_matrix();

  void recreate_shadow_tex_framebuffer();

  std::vector<glm::vec4>
  calc_frustum_corners(const ViewRenderInfo &view_render_info,
                       const glm::mat4      &projection_matrix) const;

  glm::mat4 calc_light_space_matrix(const DirectionalLight &directional_light,
                                    const ViewRenderInfo   &view_render_info,
                                    float                   near_plane,
                                    float                   far_plane) const;

  std::vector<glm::mat4>
  calc_light_space_matrices(const DirectionalLight &directional_light,
                            const ViewRenderInfo   &view_render_info) const;

  void calc_shadow_cascades_splits(const ViewRenderInfo &view_render_info);

  void recreate_scene_framebuffer(int width, int height);
  void recreate_debug_quad_framebuffer(int new_width, int new_height);
};
