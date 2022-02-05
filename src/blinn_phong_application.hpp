#pragma once

#include "engine/application.hpp"
#include "engine/camera.hpp"
#include "engine/directional_light.hpp"
#include "engine/gl_framebuffer.hpp"
#include "engine/gl_shader.hpp"
#include "engine/gl_texture.hpp"
#include "engine/gl_texture_array.hpp"
#include "engine/mesh.hpp"
#include "engine/point_light.hpp"
#include "engine/texture_cache.hpp"

#include <memory>

class BlinnPhongApplication : public Application
{
public:
  ~BlinnPhongApplication() override;

protected:
  void init() override;
  void on_update(float delta_time) override;
  void on_render_imgui() override;
  void render_imgui_general();
  void render_imgui_shadows();

  void on_window_framebuffer_size_callback(GLFWwindow *window,
                                           int         width,
                                           int         height) override;

  void
  on_mouse_movement_callback(GLFWwindow *window, double x, double y) override;

  void on_window_close_callback(GLFWwindow *window) override;

  void on_key_callback(GLFWwindow *window,
                       int         key,
                       int         scancode,
                       int         action,
                       int         mods) override;

private:
  struct MeshInfo
  {
    glm::mat4 model_matrix;
    Mesh     *mesh;
  };

  struct LightFrustum
  {
    float near{};
    float far{};
    float fov{};
    float ratio{};
    float point[8];
  };

  bool is_move_camera_{false};

  glm::vec3 sky_color_{30.0f, 81.0f, 92.0f};

  DirectionalLight directional_light_;
  TextureCache     texture_cache_;

  bool   is_mouse_first_move_{true};
  double mouse_last_x_{0.0};
  double mouse_last_y_{0.0};

  Camera camera_;
  float  camera_near_{0.1f};
  float  camera_far_{200.0f};

  glm::mat4 projection_matrix_{1.0f};

  std::vector<PointLight> point_lights_;

  std::shared_ptr<GlShader> model_shader_;

  std::vector<std::shared_ptr<Model>> models_;
  std::vector<MeshInfo>               transparent_meshes_;
  std::vector<MeshInfo>               solid_meshes_;

  int window_width_{0};
  int window_height_{0};

  std::shared_ptr<GlTexture> white_texture_{};

  std::shared_ptr<GlShader>       shadow_map_shader_{};
  std::shared_ptr<GlShader>       shadow_map_transparent_shader_{};
  std::shared_ptr<GlTextureArray> shadow_tex_array_{};
  bool                            is_shadows_enabled_{true};
  bool                            show_shadow_cascades_{false};
  float                           light_size_{10.25f};
  float                           shadow_bias_min_{0.0f};
  bool                            smooth_shadows_{true};

  GLuint                    quad_vertex_array_{};
  std::shared_ptr<GlShader>      depth_debug_shader_{};
  std::shared_ptr<GlFramebuffer> debug_quad_framebuffer_{};
  int                            debug_quad_height_{0};
  int                            debug_quad_width_{0};
  std::size_t                    debug_selected_cascade_{0};

  int                shadow_tex_width_{4096};
  int                shadow_tex_height_{4096};
  unsigned                  shadow_cascades_count_{4};
  std::vector<LightFrustum> cascade_frustums_;
  // std::vector<float> shadow_cascades_levels_{camera_far_ / 50.0f,
  //                                            camera_far_ / 25.0f,
  //                                            camera_far_ / 10.0f,
  //                                            camera_far_ / 2.0f};

  std::shared_ptr<GlFramebuffer> shadow_framebuffer_{};

  float                          exposure_{0.01f};
  std::shared_ptr<GlShader>      hdr_shader_{};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_{};

  void                   recalculate_projection_matrix();
  std::vector<glm::vec4>
            calc_frustum_corners(const glm::mat4 &projection_matrix) const;
  glm::mat4 calc_light_space_matrix(float near_plane, float far_plane) const;
  std::vector<glm::mat4> calc_light_space_matrices() const;

  void add_model(std::shared_ptr<Model> model);
  void set_move_camera(bool value);
  void move_camera(float delta_time);

  void recreate_scene_framebuffer();
  void recreate_debug_quad_framebuffer(int new_width, int new_height);

  void calc_shadow_cascades_splits();
};
