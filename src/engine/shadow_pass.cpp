#include "shadow_pass.hpp"
#include "defer.hpp"
#include "gl_texture_array.hpp"
#include "log.hpp"

namespace dc
{

ShadowPass::ShadowPass()
{
  init_shaders();
  recreate_shadow_tex_framebuffer();
}

void ShadowPass::generate_point_light_shadows(
    const SceneRenderInfo &scene_render_info)
{

  const auto &point_lights = scene_render_info.point_lights();
  for (std::size_t i = 0; i < point_lights.size(); ++i)
  {
    const auto &point_light = point_lights[i];
    if (!point_light.cast_shadow())
    {
      continue;
    }

    const auto shadow_tex = point_light.shadow_tex();

    if (!point_light_framebuffer_)
    {
      FramebufferConfig framebuffer_config{};
      framebuffer_config.depth_attachment_ = shadow_tex;
      point_light_framebuffer_             = std::make_shared<GlFramebuffer>();
      point_light_framebuffer_->attach(framebuffer_config);
    }
    else
    {
      point_light_framebuffer_->set_depth_attachment(shadow_tex);
    }

    static const float aspect{1.0f};
    static const float near{0.1f};
    const auto far = point_light.radius();

    const auto shadow_proj =
        glm::perspective(glm::radians(90.0f), aspect, near, far);

    const auto             light_pos = point_light.position();
    std::vector<glm::mat4> shadow_transforms;

    shadow_transforms.push_back(
        shadow_proj * glm::lookAt(light_pos,
                                  light_pos + glm::vec3(1.0, 0.0, 0.0),
                                  glm::vec3(0.0, -1.0, 0.0)));
    shadow_transforms.push_back(
        shadow_proj * glm::lookAt(light_pos,
                                  light_pos + glm::vec3(-1.0, 0.0, 0.0),
                                  glm::vec3(0.0, -1.0, 0.0)));
    shadow_transforms.push_back(
        shadow_proj * glm::lookAt(light_pos,
                                  light_pos + glm::vec3(0.0, 1.0, 0.0),
                                  glm::vec3(0.0, 0.0, 1.0)));
    shadow_transforms.push_back(
        shadow_proj * glm::lookAt(light_pos,
                                  light_pos + glm::vec3(0.0, -1.0, 0.0),
                                  glm::vec3(0.0, 0.0, -1.0)));
    shadow_transforms.push_back(
        shadow_proj * glm::lookAt(light_pos,
                                  light_pos + glm::vec3(0.0, 0.0, 1.0),
                                  glm::vec3(0.0, -1.0, 0.0)));
    shadow_transforms.push_back(
        shadow_proj * glm::lookAt(light_pos,
                                  light_pos + glm::vec3(0.0, 0.0, -1.0),
                                  glm::vec3(0.0, -1.0, 0.0)));

    glViewport(0, 0, PointLight::shadow_map_size, PointLight::shadow_map_size);
    point_light_framebuffer_->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    point_light_shadow_map_shader_->bind();
    for (unsigned i = 0; i < 6; ++i)
    {
      point_light_shadow_map_shader_->set_uniform("shadowMatrices[0]",
                                                  shadow_transforms);
    }

    point_light_shadow_map_shader_->set_uniform("far_plane", far);
    point_light_shadow_map_shader_->set_uniform("lightPos", light_pos);

    // iterate through all solid meshes
    for (const auto &mesh_info : solid_meshes_)
    {
      point_light_shadow_map_shader_->set_uniform("model",
                                                  mesh_info.model_matrix_);

      draw(*mesh_info.mesh_->vertex_array(), GL_TRIANGLES);
    }

    point_light_shadow_map_shader_->unbind();
    point_light_framebuffer_->unbind();
  }
}

void ShadowPass::execute(const SceneRenderInfo &scene_render_info,
                         const ViewRenderInfo & view_render_info)
{
  // sort in transparent and solid meshes
  const auto &meshes = scene_render_info.meshes();

  const auto estimated_meshes_count = meshes.size() * 10;
  solid_meshes_.reserve(estimated_meshes_count);
  transparent_meshes_.reserve(estimated_meshes_count);
  defer(solid_meshes_.clear());
  defer(transparent_meshes_.clear());

  // sort into solid and transparent geometry
  for (const auto &mesh : meshes)
  {
    if (mesh.mesh_->material()->is_transparent())
    {
      transparent_meshes_.push_back(mesh);
    }
    else
    {
      solid_meshes_.push_back(mesh);
    }
  }

  generate_point_light_shadows(scene_render_info);

  calc_shadow_cascades_splits(view_render_info);
  const auto light_space_matrices =
      calc_light_space_matrices(scene_render_info.directional_light(),
                                view_render_info);

  shadow_framebuffer_->bind();

  glViewport(0, 0, shadow_tex_width_, shadow_tex_height_);
  glClear(GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_FRONT);
  shadow_map_shader_->bind();

  shadow_map_shader_->set_uniform("light_space_matrices[0]",
                                  light_space_matrices);

  // iterate through all solid meshes
  for (const auto &mesh_info : solid_meshes_)
  {
    shadow_map_shader_->set_uniform("model_matrix", mesh_info.model_matrix_);

    draw(*mesh_info.mesh_->vertex_array(), GL_TRIANGLES);
  }

  shadow_map_shader_->unbind();

  shadow_map_transparent_shader_->bind();

  // set light space matrices
  shadow_map_transparent_shader_->set_uniform("light_space_matrices[0]",
                                              light_space_matrices);

  glCullFace(GL_BACK);
  // iterate through all transparent meshes
  for (const auto &mesh_info : transparent_meshes_)
  {
    const auto diffuse_tex = mesh_info.mesh_->material()->albedo_texture();
    if (!diffuse_tex)
    {
      continue;
    }
    // TODO: why does 1 work and 0 not?
    diffuse_tex->bind_unit(1);
    shadow_map_transparent_shader_->set_uniform("tex", 1);
    shadow_map_transparent_shader_->set_uniform("model_matrix",
                                                mesh_info.model_matrix_);

    draw(*mesh_info.mesh_->vertex_array(), GL_TRIANGLES);
  }

  shadow_map_transparent_shader_->unbind();

  shadow_framebuffer_->unbind();

  if (output_)
  {
    output_(scene_render_info,
            view_render_info,
            shadow_tex_array_,
            light_space_matrices,
            cascade_frustums_);
  }
}

void ShadowPass::calc_shadow_cascades_splits(
    const ViewRenderInfo &view_render_info)
{
  DC_ASSERT(shadow_cascades_count_ > 0, "Shadow cascades too small");
  cascade_frustums_.clear();
  cascade_frustums_.resize(shadow_cascades_count_);

  const auto far    = view_render_info.far_plane();
  const auto near   = view_render_info.near_plane();
  const auto lambda = 0.75f;
  const auto ratio  = far / near;

  cascade_frustums_[0].near = near;

  for (std::size_t i = 1; i < cascade_frustums_.size(); ++i)
  {
    const auto si = i / static_cast<float>(cascade_frustums_.size());

    cascade_frustums_[i].near = lambda * (near * glm::pow(ratio, si)) +
                                (1 - lambda) * (near + (far - near) * si);

    cascade_frustums_[static_cast<int>(i) - 1].far =
        cascade_frustums_[i].near * 1.005f;
  }
  cascade_frustums_[cascade_frustums_.size() - 1].far = far;
}

std::vector<glm::mat4> ShadowPass::calc_light_space_matrices(
    const DirectionalLight &directional_light,
    const ViewRenderInfo &  view_render_info) const
{
  std::vector<glm::mat4> matrices;
  for (const auto &frustum : cascade_frustums_)
  {
    matrices.push_back(calc_light_space_matrix(directional_light,
                                               view_render_info,
                                               frustum.near,
                                               frustum.far));
  }

  return matrices;
}

glm::mat4
ShadowPass::calc_light_space_matrix(const DirectionalLight &directional_light,
                                    const ViewRenderInfo &  view_render_info,
                                    float                   near_plane,
                                    float                   far_plane) const
{
  const auto &viewport = view_render_info.viewport_info();
  const auto  projection =
      glm::perspective(glm::radians(view_render_info.fov()),
                       static_cast<float>(viewport.width_) /
                           static_cast<float>(viewport.height_),
                       near_plane,
                       far_plane);

  const auto frustum_corners =
      calc_frustum_corners(view_render_info, projection);

  glm::vec3 center{0.0f};
  for (const auto &corner : frustum_corners)
  {
    center += glm::vec3{corner};
  }
  center /= frustum_corners.size();

  const auto light_direction = directional_light.direction();
  glm::vec3  up{0.0f, 1.0f, 0.0f};
  if (glm::abs(glm::abs(glm::dot(up, light_direction)) - 1.0f) < 0.0001f)
  {
    up = glm::vec3{0.0f, 0.0f, 1.0f};
  }
  const auto light_view = glm::lookAt(center - light_direction, center, up);

  auto min_x = std::numeric_limits<float>::max();
  auto max_x = std::numeric_limits<float>::min();
  auto min_y = std::numeric_limits<float>::max();
  auto max_y = std::numeric_limits<float>::min();
  auto min_z = std::numeric_limits<float>::max();
  auto max_z = std::numeric_limits<float>::min();

  for (const auto &corner : frustum_corners)
  {
    const auto trf = light_view * corner;

    min_x = std::min(min_x, trf.x);
    max_x = std::max(max_x, trf.x);
    min_y = std::min(min_y, trf.y);
    max_y = std::max(max_y, trf.y);
    min_z = std::min(min_z, trf.z);
    max_z = std::max(max_z, trf.z);
  }

  // tune this parameter according to the scene
  constexpr auto z_mult = 10.0f;
  if (min_z < 0)
  {
    min_z *= z_mult;
  }
  else
  {
    min_z /= z_mult;
  }

  if (max_z < 0)
  {
    max_z /= z_mult;
  }
  else
  {
    max_z *= z_mult;
  }

  const auto light_projection =
      glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);

  return light_projection * light_view;
}

std::vector<glm::vec4>
ShadowPass::calc_frustum_corners(const ViewRenderInfo &view_render_info,
                                 const glm::mat4 &     projection_matrix) const
{
  const auto inv =
      glm::inverse(projection_matrix * view_render_info.view_matrix());

  std::vector<glm::vec4> frustum_corners;

  for (unsigned x = 0; x < 2; ++x)
  {
    for (unsigned y = 0; y < 2; ++y)
    {
      for (unsigned z = 0; z < 2; ++z)
      {
        const auto point =
            inv *
            glm::vec4{2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f};
        frustum_corners.push_back(point / point.w);
      }
    }
  }

  return frustum_corners;
}

void ShadowPass::recreate_shadow_tex_framebuffer()
{
  GlTextureArrayConfig texture_array_data{
      GL_DEPTH_COMPONENT32F,
      GL_DEPTH_COMPONENT,
      shadow_tex_width_,
      shadow_tex_height_,
      static_cast<int>(shadow_cascades_count_)};
  texture_array_data.min_filter   = GL_NEAREST;
  texture_array_data.mag_filter   = GL_NEAREST;
  texture_array_data.type         = GL_FLOAT;
  texture_array_data.border_color = {1.0f, 1.0f, 1.0f, 1.0f};
  shadow_tex_array_ = std::make_shared<GlTextureArray>(texture_array_data);

  shadow_framebuffer_ = std::make_shared<GlFramebuffer>();

  FramebufferConfig framebuffer_config{};
  framebuffer_config.depth_attachment_ = shadow_tex_array_;
  shadow_framebuffer_->attach(framebuffer_config);
}

void ShadowPass::init_shaders()
{
  shadow_map_shader_ = std::make_shared<GlShader>();
  shadow_map_shader_->init("shaders/shadow_map.vert",
                           "shaders/shadow_map.geom",
                           "shaders/shadow_map.frag");

  shadow_map_transparent_shader_ = std::make_shared<GlShader>();
  shadow_map_transparent_shader_->init("shaders/shadow_map_transparent.vert",
                                       "shaders/shadow_map_transparent.geom",
                                       "shaders/shadow_map_transparent.frag");

  point_light_shadow_map_shader_ = std::make_shared<GlShader>();
  point_light_shadow_map_shader_->init(
      "shaders/learnopengl/point_light_shadow_map.vert",
      "shaders/learnopengl/point_light_shadow_map.geom",
      "shaders/learnopengl/point_light_shadow_map.frag");
}

void ShadowPass::set_output(Output output) { output_ = output; }

} // namespace dc
