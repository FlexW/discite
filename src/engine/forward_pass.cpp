#include "forward_pass.hpp"
#include "gl_texture_array.hpp"
#include "render_pass.hpp"
#include "shadow_pass.hpp"

#include <memory>

namespace dc
{

ForwardPass::ForwardPass()
{
  // dummy/placeholder texture
  white_texture_ = std::make_shared<GlTexture>();
  std::vector<unsigned char> white_tex_data{255};
  white_texture_->set_data(white_tex_data.data(), 1, 1, 1, false);

  brdf_lut_texture_ = std::make_shared<GlTexture>();
  brdf_lut_texture_->load_from_file("data/brdf_lut.ktx", true);

  init_shaders();

  // for skybox
  glDepthFunc(GL_LEQUAL);
}

void ForwardPass::execute(const SceneRenderInfo          &scene_render_info,
                          const ViewRenderInfo           &view_render_info,
                          std::shared_ptr<GlTextureArray> shadow_tex_array,
                          const std::vector<glm::mat4>    light_space_matrices,
                          const std::vector<CascadeSplit> cascade_frustums)
{
  const auto &viewport_info = view_render_info.viewport_info();
  recreate_scene_framebuffer(viewport_info.width_, viewport_info.height_);

  scene_framebuffer_->bind();
  glCullFace(GL_BACK);

  int global_texture_slot{0};

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glViewport(0, 0, viewport_info.width_, viewport_info.height_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  const auto view_matrix = view_render_info.view_matrix();

  mesh_shader_->bind();
  mesh_shader_->set_uniform("view_matrix", view_matrix);
  mesh_shader_->set_uniform("projection_matrix",
                            view_render_info.projection_matrix());

  glActiveTexture(GL_TEXTURE0 + global_texture_slot);
  brdf_lut_texture_->bind();
  mesh_shader_->set_uniform("brdf_lut_tex", global_texture_slot);
  ++global_texture_slot;

  const auto &sky = scene_render_info.env_map();
  glActiveTexture(GL_TEXTURE0 + global_texture_slot);
  sky.env_texture()->bind();
  mesh_shader_->set_uniform("env_tex", global_texture_slot);
  ++global_texture_slot;

  glActiveTexture(GL_TEXTURE0 + global_texture_slot);
  sky.env_irradiance_texture()->bind();
  mesh_shader_->set_uniform("env_irradiance_tex", global_texture_slot);
  ++global_texture_slot;

  const int   max_point_light_count{5};
  const auto &point_lights      = scene_render_info.point_lights();
  const auto  point_light_count = point_lights.size() <= max_point_light_count
                                      ? static_cast<int>(point_lights.size())
                                      : max_point_light_count;
  mesh_shader_->set_uniform("point_light_count", point_light_count);
  for (int i = 0; i < point_light_count; ++i)
  {
    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) + "].color",
                              point_lights[i].color());
    mesh_shader_->set_uniform(
        "point_lights[" + std::to_string(i) + "].position",
        glm::vec3(view_matrix * glm::vec4(point_lights[i].position(), 1.0f)));
    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) + "].linear",
                              point_lights[i].linear());
    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                  "].constant",
                              point_lights[i].constant());
    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                  "].quadratic",
                              point_lights[i].quadratic());
  }

  const auto &directional_light = scene_render_info.directional_light();
  mesh_shader_->set_uniform("smooth_shadows", smooth_shadows_);
  mesh_shader_->set_uniform("shadow_bias_min", shadow_bias_min_);
  mesh_shader_->set_uniform("light_size", light_size_);
  mesh_shader_->set_uniform("directional_light_enabled", true);
  mesh_shader_->set_uniform(
      "directional_light.direction",
      glm::vec3{view_matrix * glm::vec4{directional_light.direction(), 0.0f}});
  mesh_shader_->set_uniform("directional_light.color",
                            directional_light.color());

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
    for (std::size_t i = 0; i < cascade_frustums.size(); ++i)
    {
      mesh_shader_->set_uniform("cascades_plane_distances[" +
                                    std::to_string(i) + "]",
                                cascade_frustums[i].far);
    }

    glActiveTexture(GL_TEXTURE0 + global_texture_slot);
    shadow_tex_array->bind();
    mesh_shader_->set_uniform("directional_light_shadow_tex",
                              global_texture_slot);
    ++global_texture_slot;
  }

  // iterate through all meshes
  for (const auto &mesh : scene_render_info.meshes())
  {
    mesh_shader_->set_uniform("model_matrix", mesh.model_matrix_);

    int texture_slot = global_texture_slot;

    const auto material = mesh.mesh_->material();
    if (material->albedo_texture())
    {
      const auto albedo_texture = material->albedo_texture();
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      albedo_texture->bind();
      mesh_shader_->set_uniform("in_albedo_tex", texture_slot);
      mesh_shader_->set_uniform("albedo_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      white_texture_->bind();
      mesh_shader_->set_uniform("in_albedo_tex", texture_slot);
      mesh_shader_->set_uniform("in_albedo_color", material->albedo_color());
      mesh_shader_->set_uniform("albedo_tex_enabled", false);
    }

    if (material->roughness_texture())
    {
      const auto roughness_texture = material->roughness_texture();
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      roughness_texture->bind();
      mesh_shader_->set_uniform("in_roughness_tex", texture_slot);
      mesh_shader_->set_uniform("roughness_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      white_texture_->bind();
      mesh_shader_->set_uniform("in_roughness_tex", texture_slot);
      mesh_shader_->set_uniform("in_roughness", material->roughness());
      mesh_shader_->set_uniform("roughness_tex_enabled", false);
    }

    if (material->ambient_occlusion_texture())
    {
      const auto ao_texture = material->ambient_occlusion_texture();
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      ao_texture->bind();
      mesh_shader_->set_uniform("in_ao_tex", texture_slot);
      mesh_shader_->set_uniform("ao_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      white_texture_->bind();
      mesh_shader_->set_uniform("in_ao_tex", texture_slot);
      mesh_shader_->set_uniform("ao_tex_enabled", false);
    }

    if (material->emissive_texture())
    {
      const auto emissive_texture = material->emissive_texture();
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      emissive_texture->bind();
      mesh_shader_->set_uniform("in_emissive_tex", texture_slot);
      mesh_shader_->set_uniform("emissive_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      white_texture_->bind();
      mesh_shader_->set_uniform("in_emissive_tex", texture_slot);
      mesh_shader_->set_uniform("emissive_tex_enabled", false);
      mesh_shader_->set_uniform("in_emissive_color",
                                material->emissive_color());
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

    const auto vertex_array = mesh.mesh_->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }

  mesh_shader_->unbind();
  scene_framebuffer_->unbind();

  if (output_)
  {
    output_(scene_render_info, view_render_info, scene_framebuffer_);
  }
}

void ForwardPass::init_shaders()
{
  mesh_shader_ = std::make_shared<GlShader>();
  mesh_shader_->init("shaders/pbr.vert", "shaders/pbr.frag");
}

void ForwardPass::recreate_scene_framebuffer(int width, int height)
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

void ForwardPass::set_output(Output output) { output_ = output; }

} // namespace dc
