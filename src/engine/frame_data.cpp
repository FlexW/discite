#include "frame_data.hpp"
#include "gl_framebuffer.hpp"

namespace dc
{

void SceneRenderInfo::add_mesh(const MeshInfo &mesh_info)
{
  meshes_.push_back(mesh_info);
}

std::vector<MeshInfo> SceneRenderInfo::meshes() const { return meshes_; }

void SceneRenderInfo::add_point_light(const PointLight &point_light)
{
  point_lights_.push_back(point_light);
}

std::vector<PointLight> SceneRenderInfo::point_lights() const
{
  return point_lights_;
}

void SceneRenderInfo::set_directional_light(
    const DirectionalLight &directional_light)
{
  directional_light_ = directional_light;
}

DirectionalLight SceneRenderInfo::directional_light() const
{
  return directional_light_;
}

void SceneRenderInfo::set_env_map(const EnvionmentMap &value)
{
  env_map_ = value;
}

EnvionmentMap SceneRenderInfo::env_map() const { return env_map_; }

void ViewRenderInfo::set_view_matrix(const glm::mat4 &value)
{
  view_matrix_ = value;
}

glm::mat4 ViewRenderInfo::view_matrix() const { return view_matrix_; }

void ViewRenderInfo::set_projection_matrix(const glm::mat4 &value)
{
  projection_matrix_ = value;
}

glm::mat4 ViewRenderInfo::projection_matrix() const
{
  return projection_matrix_;
}

void ViewRenderInfo::set_viewport_info(const ViewportInfo &value)
{
  viewport_info_ = value;
}

ViewportInfo ViewRenderInfo::viewport_info() const { return viewport_info_; }

void ViewRenderInfo::set_aspect_ratio(float value) { aspect_ratio_ = value; }

float ViewRenderInfo::aspect_ratio() const { return aspect_ratio_; }

void ViewRenderInfo::set_near_plane(float value) { near_plane_ = value; }

float ViewRenderInfo::near_plane() const { return near_plane_; }

void ViewRenderInfo::set_far_plane(float value) { far_plane_ = value; }

float ViewRenderInfo::far_plane() const { return far_plane_; }

void ViewRenderInfo::set_fov(float value) { fov_ = value; }

float ViewRenderInfo::fov() const { return fov_; }

void ViewRenderInfo::set_framebuffer(GlFramebuffer *framebuffer)
{
  if (framebuffer != nullptr)
  {
    framebuffer_ = framebuffer;
  }
  else
  {
    framebuffer_ = {};
  }
}

std::optional<GlFramebuffer *> ViewRenderInfo::framebuffer() const
{
  return framebuffer_;
}

} // namespace dc