#pragma once

#include "directional_light.hpp"
#include "environment_map.hpp"
#include "gl_framebuffer.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "point_light.hpp"
#include <optional>

namespace dc
{

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

  void                           set_framebuffer(GlFramebuffer *framebuffer);
  std::optional<GlFramebuffer *> framebuffer() const;

private:
  float        fov_{};
  float        aspect_ratio_{};
  float        near_plane_{};
  float        far_plane_{};
  ViewportInfo viewport_info_{};
  glm::mat4    view_matrix_{1.0f};
  glm::mat4    projection_matrix_{1.0f};

  std::optional<GlFramebuffer *> framebuffer_;
};

} // namespace dc
