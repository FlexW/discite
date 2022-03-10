#include "forward_pass.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"
#include "log.hpp"
#include "render_pass.hpp"
#include "shadow_pass.hpp"

#include <algorithm>
#include <iterator>
#include <memory>

namespace dc
{

ForwardPass::ForwardPass()
{
  // dummy/placeholder texture
  std::vector<unsigned char> white_tex_data{255};
  GlTextureConfig            white_tex_config{};
  white_tex_config.data_             = white_tex_data.data();
  white_tex_config.width_            = 1;
  white_tex_config.height_           = 1;
  white_tex_config.format_           = GL_RED;
  white_tex_config.sized_format_     = GL_R8;
  white_tex_config.generate_mipmaps_ = false;
  white_texture_ = std::make_shared<GlTexture>(white_tex_config);

  brdf_lut_texture_ = GlTexture::load_from_file("data/brdf_lut.ktx");

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

  glViewport(0, 0, viewport_info.width_, viewport_info.height_);
  constexpr std::array<float, 4> clear_color{0.0f, 0.0f, 0.0f, 1.0f};
  glClearNamedFramebufferfv(scene_framebuffer_->id(),
                            GL_COLOR,
                            0,
                            clear_color.data());
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  const auto view_matrix = view_render_info.view_matrix();

  mesh_shader_->bind();
  mesh_shader_->set_uniform("view_matrix", view_matrix);
  mesh_shader_->set_uniform("projection_matrix",
                            view_render_info.projection_matrix());

  brdf_lut_texture_->bind_unit(global_texture_slot);
  mesh_shader_->set_uniform("brdf_lut_tex", global_texture_slot);
  ++global_texture_slot;

  const auto &sky = scene_render_info.env_map();
  sky.env_texture()->bind_unit(global_texture_slot);
  mesh_shader_->set_uniform("env_tex", global_texture_slot);
  ++global_texture_slot;

  sky.env_irradiance_texture()->bind_unit(global_texture_slot);
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
    mesh_shader_->set_uniform("light_space_matrices[0]", light_space_matrices);

    mesh_shader_->set_uniform("show_shadow_cascades", show_shadow_cascades_);
    std::vector<float> far_planes;
    std::transform(cascade_frustums.cbegin(),
                   cascade_frustums.cend(),
                   std::back_inserter(far_planes),
                   [](const auto &frustum) { return frustum.far; });
    mesh_shader_->set_uniform("cascades_plane_distances[0]", far_planes);

    shadow_tex_array->bind_unit(global_texture_slot);
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
      albedo_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_albedo_tex", texture_slot);
      mesh_shader_->set_uniform("albedo_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_albedo_tex", texture_slot);
      mesh_shader_->set_uniform("in_albedo_color", material->albedo_color());
      mesh_shader_->set_uniform("albedo_tex_enabled", false);
    }

    if (material->roughness_texture())
    {
      const auto roughness_texture = material->roughness_texture();
      roughness_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_roughness_tex", texture_slot);
      mesh_shader_->set_uniform("roughness_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_roughness_tex", texture_slot);
      mesh_shader_->set_uniform("in_roughness", material->roughness());
      mesh_shader_->set_uniform("roughness_tex_enabled", false);
    }

    if (material->ambient_occlusion_texture())
    {
      const auto ao_texture = material->ambient_occlusion_texture();
      ao_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_ao_tex", texture_slot);
      mesh_shader_->set_uniform("ao_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_ao_tex", texture_slot);
      mesh_shader_->set_uniform("ao_tex_enabled", false);
    }

    if (material->emissive_texture())
    {
      const auto emissive_texture = material->emissive_texture();
      emissive_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_emissive_tex", texture_slot);
      mesh_shader_->set_uniform("emissive_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_emissive_tex", texture_slot);
      mesh_shader_->set_uniform("emissive_tex_enabled", false);
      mesh_shader_->set_uniform("in_emissive_color",
                                material->emissive_color());
    }

    if (material->normal_texture())
    {
      const auto normal_texture = material->normal_texture();
      normal_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_normal_tex", texture_slot);
      mesh_shader_->set_uniform("normal_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
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
