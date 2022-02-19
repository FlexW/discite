#include "renderer.hpp"
#include "defer.hpp"
#include "directional_light.hpp"
#include "gl_framebuffer.hpp"

#include <optional>

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

Renderer::Renderer()
{
  load_shaders();
  recreate_shadow_tex_framebuffer();

  glGenVertexArrays(1, &quad_vertex_array_);

  // dummy/placeholder texture
  white_texture_ = std::make_shared<GlTexture>();
  std::vector<unsigned char> white_tex_data{255};
  white_texture_->set_data(white_tex_data.data(), 1, 1, 1, false);
}

Renderer::~Renderer()
{
  if (quad_vertex_array_)
  {
    glDeleteVertexArrays(1, &quad_vertex_array_);
  }
}

void Renderer::submit(const SceneRenderInfo         &scene_render_info,
                      const ViewRenderInfo          &view_render_info,
                      std::optional<GlFramebuffer *> framebuffer)
{
  const auto &viewport_info = view_render_info.viewport_info();
  recreate_scene_framebuffer(viewport_info.width_, viewport_info.height_);

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

  calc_shadow_cascades_splits(view_render_info);
  const auto light_space_matrices =
      calc_light_space_matrices(scene_render_info.directional_light(),
                                view_render_info);
  // render the shadow map
  if (is_shadows_enabled_)
  {
    shadow_framebuffer_->bind();

    glViewport(0, 0, shadow_tex_width_, shadow_tex_height_);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    shadow_map_shader_->bind();

    // set light space matrices
    for (std::size_t i = 0; i < light_space_matrices.size(); ++i)
    {
      shadow_map_shader_->set_uniform("light_space_matrices[" +
                                          std::to_string(i) + "]",
                                      light_space_matrices[i]);
    }

    // iterate through all solid meshes
    for (const auto &mesh_info : solid_meshes_)
    {
      shadow_map_shader_->set_uniform("model_matrix", mesh_info.model_matrix_);

      draw(*mesh_info.mesh_->vertex_array(), GL_TRIANGLES);
    }

    shadow_map_shader_->unbind();

    shadow_map_transparent_shader_->bind();

    // set light space matrices
    for (std::size_t i = 0; i < light_space_matrices.size(); ++i)
    {
      shadow_map_transparent_shader_->set_uniform("light_space_matrices[" +
                                                      std::to_string(i) + "]",
                                                  light_space_matrices[i]);
    }

    glCullFace(GL_BACK);
    // iterate through all transparent meshes
    for (const auto &mesh_info : transparent_meshes_)
    {
      const auto diffuse_tex = mesh_info.mesh_->material()->diffuse_texture();
      if (!diffuse_tex)
      {
        continue;
      }
      // TODO: why does 1 work and 0 not?
      glActiveTexture(GL_TEXTURE1);
      diffuse_tex->bind();
      shadow_map_transparent_shader_->set_uniform("tex", 1);
      shadow_map_transparent_shader_->set_uniform("model_matrix",
                                                  mesh_info.model_matrix_);

      draw(*mesh_info.mesh_->vertex_array(), GL_TRIANGLES);
    }

    shadow_map_transparent_shader_->unbind();

    shadow_framebuffer_->unbind();
  }

  // render the scene
  {
    scene_framebuffer_->bind();
    glCullFace(GL_BACK);

    int global_texture_slot = 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glViewport(0, 0, viewport_info.width_, viewport_info.height_);
    glClearColor(sky_color_.r, sky_color_.g, sky_color_.b, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    const auto view_matrix = view_render_info.view_matrix();

    mesh_shader_->bind();
    mesh_shader_->set_uniform("view_matrix", view_matrix);
    mesh_shader_->set_uniform("projection_matrix",
                              view_render_info.projection_matrix());

    const int   max_point_light_count{5};
    const auto &point_lights      = scene_render_info.point_lights();
    const auto  point_light_count = point_lights.size() <= max_point_light_count
                                        ? static_cast<int>(point_lights.size())
                                        : max_point_light_count;
    mesh_shader_->set_uniform("point_light_count", point_light_count);
    for (int i = 0; i < point_light_count; ++i)
    {
      mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                    "].ambient_color",
                                point_lights[i].ambient_color());
      mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                    "].diffuse_color",
                                point_lights[i].diffuse_color());
      mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                    "].specular_color",
                                point_lights[i].specular_color());
      mesh_shader_->set_uniform(
          "point_lights[" + std::to_string(i) + "].position",
          glm::vec3(view_matrix * glm::vec4(point_lights[i].position(), 1.0f)));
      mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                    "].linear",
                                point_lights[i].linear());
      mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                    "].constant",
                                point_lights[i].constant());
      mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                    "].quadratic",
                                point_lights[i].quadratic());
    }

    mesh_shader_->set_uniform("spot_light_count", 0);

    const auto &directional_light = scene_render_info.directional_light();
    mesh_shader_->set_uniform("smooth_shadows", smooth_shadows_);
    mesh_shader_->set_uniform("shadow_bias_min", shadow_bias_min_);
    mesh_shader_->set_uniform("light_size", light_size_);
    mesh_shader_->set_uniform("directional_light_enabled", true);
    mesh_shader_->set_uniform("directional_light.direction_ws",
                              directional_light.direction());
    mesh_shader_->set_uniform(
        "directional_light.direction",
        glm::vec3{view_matrix *
                  glm::vec4{directional_light.direction(), 0.0f}});
    mesh_shader_->set_uniform("directional_light.ambient_color",
                              directional_light.ambient_color());
    mesh_shader_->set_uniform("directional_light.diffuse_color",
                              directional_light.diffuse_color());
    mesh_shader_->set_uniform("directional_light.specular_color",
                              directional_light.specular_color());

    mesh_shader_->set_uniform("directional_light_shadow_enabled",
                              is_shadows_enabled_);

    {
      for (std::size_t i = 0; i < light_space_matrices.size(); ++i)
      {
        mesh_shader_->set_uniform("light_space_matrices[" + std::to_string(i) +
                                      "]",
                                  light_space_matrices[i]);
      }

      mesh_shader_->set_uniform("show_shadow_cascades", show_shadow_cascades_);
      for (std::size_t i = 0; i < shadow_cascades_count_; ++i)
      {
        mesh_shader_->set_uniform("cascades_plane_distances[" +
                                      std::to_string(i) + "]",
                                  cascade_frustums_[i].far);
      }

      glActiveTexture(GL_TEXTURE0 + global_texture_slot);
      shadow_tex_array_->bind();
      mesh_shader_->set_uniform("directional_light_shadow_tex",
                                global_texture_slot);
      ++global_texture_slot;
    }

    // iterate through all meshes
    for (const auto &mesh : meshes)
    {
      mesh_shader_->set_uniform("model_matrix", mesh.model_matrix_);

      int texture_slot = global_texture_slot;

      const auto material = mesh.mesh_->material();
      if (material->diffuse_texture())
      {
        const auto diffuse_texture = material->diffuse_texture();
        glActiveTexture(GL_TEXTURE0 + texture_slot);
        diffuse_texture->bind();
        mesh_shader_->set_uniform("in_diffuse_tex", texture_slot);
        mesh_shader_->set_uniform("diffuse_tex_enabled", true);
        ++texture_slot;
      }
      else
      {
        glActiveTexture(GL_TEXTURE0 + texture_slot);
        white_texture_->bind();
        mesh_shader_->set_uniform("in_diffuse_tex", texture_slot);
        mesh_shader_->set_uniform("in_diffuse_color",
                                  material->diffuse_color());
        mesh_shader_->set_uniform("diffuse_tex_enabled", false);
      }

      if (material->normal_texture())
      {
        const auto normal_texture = material->normal_texture();
        glActiveTexture(GL_TEXTURE0 + texture_slot);
        normal_texture->bind();
        mesh_shader_->set_uniform("in_normal_tex", texture_slot);
        mesh_shader_->set_uniform("normal_tex_enabled", true);
        ++texture_slot;
      }
      else
      {
        glActiveTexture(GL_TEXTURE0 + texture_slot);
        white_texture_->bind();
        mesh_shader_->set_uniform("in_normal_tex", texture_slot);
        mesh_shader_->set_uniform("normal_tex_enabled", false);
      }

      mesh_shader_->set_uniform("specular_power", material->specular_power());

      const auto vertex_array = mesh.mesh_->vertex_array();
      draw(*vertex_array, GL_TRIANGLES);
    }

    mesh_shader_->unbind();
    scene_framebuffer_->unbind();
  }

  // perform hdr to sdr conversation
  {
    if (framebuffer.has_value())
    {
      // render in the user submitted framebuffer
      framebuffer.value()->bind();
    }
    else
    {
      // render to the windows default framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glViewport(0, 0, viewport_info.width_, viewport_info.height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    hdr_shader_->bind();

    const auto scene_tex = std::get<std::shared_ptr<GlTexture>>(
        scene_framebuffer_->color_attachment(0));
    glActiveTexture(GL_TEXTURE0);
    scene_tex->bind();
    hdr_shader_->set_uniform("exposure", exposure_);
    hdr_shader_->set_uniform("hdr_tex", 0);

    // draw quad
    glBindVertexArray(quad_vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    hdr_shader_->unbind();

    // bind default framebuffer in any case
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void Renderer::load_shaders()
{
  mesh_shader_ = std::make_shared<GlShader>();
  mesh_shader_->init("shaders/blinn_phong.vert", "shaders/blinn_phong.frag");

  shadow_map_shader_ = std::make_shared<GlShader>();
  shadow_map_shader_->init("shaders/shadow_map.vert",
                           "shaders/shadow_map.geom",
                           "shaders/shadow_map.frag");

  shadow_map_transparent_shader_ = std::make_shared<GlShader>();
  shadow_map_transparent_shader_->init("shaders/shadow_map_transparent.vert",
                                       "shaders/shadow_map_transparent.geom",
                                       "shaders/shadow_map_transparent.frag");

  depth_debug_shader_ = std::make_shared<GlShader>();
  depth_debug_shader_->init("shaders/quad.vert", "shaders/debug_depth.frag");

  hdr_shader_ = std::make_shared<GlShader>();
  hdr_shader_->init("shaders/quad.vert", "shaders/hdr.frag");
}

void Renderer::recreate_shadow_tex_framebuffer()
{
  shadow_tex_array_ = std::make_shared<GlTextureArray>();
  TextureArrayData texture_array_data{GL_DEPTH_COMPONENT32F,
                                      GL_DEPTH_COMPONENT,
                                      shadow_tex_width_,
                                      shadow_tex_height_,
                                      static_cast<int>(shadow_cascades_count_)};
  texture_array_data.min_filter   = GL_NEAREST;
  texture_array_data.mag_filter   = GL_NEAREST;
  texture_array_data.type         = GL_FLOAT;
  texture_array_data.border_color = {1.0f, 1.0f, 1.0f, 1.0f};
  shadow_tex_array_->set_data(texture_array_data);

  shadow_framebuffer_ = std::make_shared<GlFramebuffer>();

  FramebufferConfig framebuffer_config{};
  framebuffer_config.depth_attachment_ = shadow_tex_array_;
  shadow_framebuffer_->attach(framebuffer_config);
}

void Renderer::recreate_scene_framebuffer(int width, int height)
{
  if (width == scene_framebuffer_width_ && height == scene_framebuffer_height_)
  {
    return;
  }
  scene_framebuffer_width_  = width;
  scene_framebuffer_height_ = height;

  FramebufferAttachmentCreateConfig color_config{};
  color_config.type_            = AttachmentType::Texture;
  color_config.format_          = GL_RGBA;
  color_config.internal_format_ = GL_RGBA16F;
  color_config.width_           = scene_framebuffer_width_;
  color_config.height_          = scene_framebuffer_height_;

  FramebufferAttachmentCreateConfig depth_config{};
  depth_config.type_            = AttachmentType::Renderbuffer;
  depth_config.format_          = GL_DEPTH_COMPONENT;
  depth_config.internal_format_ = GL_DEPTH_COMPONENT32F;
  depth_config.width_           = scene_framebuffer_width_;
  depth_config.height_          = scene_framebuffer_height_;

  FramebufferConfig config{};
  config.color_attachments_.push_back(color_config);
  config.depth_attachment_ = depth_config;

  scene_framebuffer_ = std::make_shared<GlFramebuffer>();
  scene_framebuffer_->attach(config);
}

std::vector<glm::vec4>
Renderer::calc_frustum_corners(const ViewRenderInfo &view_render_info,
                               const glm::mat4      &projection_matrix) const
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

glm::mat4
Renderer::calc_light_space_matrix(const DirectionalLight &directional_light,
                                  const ViewRenderInfo   &view_render_info,
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

  const auto light_view = glm::lookAt(center - directional_light.direction(),
                                      center,
                                      glm::vec3{0.0f, 1.0f, 0.0f});

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

std::vector<glm::mat4> Renderer::calc_light_space_matrices(
    const DirectionalLight &directional_light,
    const ViewRenderInfo   &view_render_info) const
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

void Renderer::calc_shadow_cascades_splits(
    const ViewRenderInfo &view_render_info)
{
  assert(shadow_cascades_count_ > 0);
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

void Renderer::recreate_debug_quad_framebuffer(int new_width, int new_height)
{
  if (debug_quad_width_ == new_width && debug_quad_height_ == new_height)
  {
    return;
  }

  debug_quad_width_  = new_width;
  debug_quad_height_ = new_height;

  FramebufferAttachmentCreateConfig color_attachment_config{};
  color_attachment_config.format_          = GL_RGB;
  color_attachment_config.internal_format_ = GL_RGB32F;
  color_attachment_config.width_           = debug_quad_width_;
  color_attachment_config.height_          = debug_quad_height_;
  color_attachment_config.type_            = AttachmentType::Texture;

  FramebufferConfig framebuffer_config{};
  framebuffer_config.color_attachments_.push_back(color_attachment_config);

  debug_quad_framebuffer_ = std::make_shared<GlFramebuffer>();
  debug_quad_framebuffer_->attach(framebuffer_config);
}
