#include "forward_pass.hpp"
#include "frame_data.hpp"
#include "gl_shader.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"
#include "gl_vertex_array.hpp"
#include "gl_vertex_buffer.hpp"
#include "log.hpp"
#include "material.hpp"
#include "render_pass.hpp"
#include "shadow_pass.hpp"

#include <stb_image.h>

#include <algorithm>
#include <iterator>
#include <memory>

namespace dc
{

ForwardPass::ForwardPass()
{
  GlVertexBufferLayout line_layout;
  line_layout.push_float(3);
  line_layout.push_float(3);
  lines_vertex_buffer_ = std::make_shared<GlVertexBuffer>(
      max_debug_lines_count * sizeof(DebugLineInfo),
      line_layout);
  lines_vertex_array_ = std::make_shared<GlVertexArray>();
  lines_vertex_array_->add_vertex_buffer(lines_vertex_buffer_);

  // dummy/placeholder texture
  std::vector<unsigned char> white_tex_data{255, 255, 255};
  GlTextureConfig            white_tex_config{};
  white_tex_config.data_             = white_tex_data.data();
  white_tex_config.width_            = 1;
  white_tex_config.height_           = 1;
  white_tex_config.format_           = GL_RGB;
  white_tex_config.sized_format_     = GL_RGB8;
  white_tex_config.generate_mipmaps_ = false;
  white_texture_ = std::make_shared<GlTexture>(white_tex_config);

  GlCubeTextureConfig dummy_cube_texture_config{};
  dummy_cube_texture_config.width_            = 1;
  dummy_cube_texture_config.height_           = 1;
  dummy_cube_texture_config.format            = GL_RED;
  dummy_cube_texture_config.sized_format      = GL_R8;
  dummy_cube_texture_config.type_             = GL_UNSIGNED_BYTE;
  dummy_cube_texture_config.min_filter_       = GL_LINEAR;
  dummy_cube_texture_config.generate_mipmaps_ = false;
  dummy_cube_texture_ =
      std::make_shared<GlCubeTexture>(dummy_cube_texture_config);

  brdf_lut_texture_ = GlTexture::load_from_file("data/brdf_lut.ktx");

  init_shaders();

  // for skybox
  glDepthFunc(GL_LEQUAL);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void         renderCube()
{
  // initialize (if necessary)
  if (cubeVAO == 0)
  {
    float vertices[] = {
        // back face
        -1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f, // bottom-left
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        1.0f, // top-right
        1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        0.0f, // bottom-right
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        1.0f, // top-right
        -1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f, // bottom-left
        -1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f, // top-left
        // front face
        -1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f, // bottom-left
        1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f, // bottom-right
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f, // top-right
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f, // top-right
        -1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f, // top-left
        -1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f, // bottom-left
        // left face
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f, // top-right
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f, // top-left
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f, // bottom-left
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f, // bottom-left
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f, // bottom-right
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f, // top-right
              // right face
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f, // top-left
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f, // bottom-right
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f, // top-right
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f, // bottom-right
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f, // top-left
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f, // bottom-left
        // bottom face
        -1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f, // top-right
        1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        1.0f, // top-left
        1.0f,
        -1.0f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f, // bottom-left
        1.0f,
        -1.0f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f, // bottom-left
        -1.0f,
        -1.0f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f, // bottom-right
        -1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f, // top-right
        // top face
        -1.0f,
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f, // top-left
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f, // bottom-right
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f, // top-right
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f, // bottom-right
        -1.0f,
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f, // top-left
        -1.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f // bottom-left
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          8 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  // render Cube
  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

void ForwardPass::set_ibl(GlShader         &shader,
                          int              &global_texture_slot,
                          const EnvMapData &env_map_data)
{
  brdf_lut_texture_->bind_unit(global_texture_slot);
  shader.set_uniform("brdf_lut_tex", global_texture_slot);
  ++global_texture_slot;

  env_map_data.prefilter_tex_->bind_unit(global_texture_slot);
  shader.set_uniform("env_tex", global_texture_slot);
  ++global_texture_slot;

  env_map_data.irradiance_tex_->bind_unit(global_texture_slot);
  shader.set_uniform("env_irradiance_tex", global_texture_slot);
  ++global_texture_slot;
}

void ForwardPass::set_lightning(
    GlShader                        &shader,
    int                             &global_texture_slot,
    const SceneRenderInfo           &scene_render_info,
    const ViewRenderInfo            &view_render_info,
    std::shared_ptr<GlTextureArray>  shadow_tex_array,
    const std::vector<glm::mat4>    &light_space_matrices,
    const std::vector<CascadeSplit> &cascade_frustums)
{
  const auto view_matrix = view_render_info.view_matrix();
  const int  max_point_light_count{30};

  const auto &point_lights      = scene_render_info.point_lights();
  const auto  point_light_count = point_lights.size() <= max_point_light_count
                                      ? static_cast<int>(point_lights.size())
                                      : max_point_light_count;

  std::vector<GLint> point_light_shadow_maps;
  shader.set_uniform("point_light_count", point_light_count);
  for (int i = 0; i < point_light_count; ++i)
  {
    shader.set_uniform(
        "point_lights[" + std::to_string(i) + "].position",
        glm::vec3(view_matrix * glm::vec4(point_lights[i].position(), 1.0f)));
    shader.set_uniform("point_lights[" + std::to_string(i) +
                           "].position_world_space",
                       point_lights[i].position());

    shader.set_uniform("point_lights[" + std::to_string(i) + "].color",
                       point_lights[i].color());
    shader.set_uniform("point_lights[" + std::to_string(i) + "].multiplier",
                       point_lights[i].multiplier());

    shader.set_uniform("point_lights[" + std::to_string(i) + "].radius",
                       point_lights[i].radius());
    shader.set_uniform("point_lights[" + std::to_string(i) + "].falloff",
                       point_lights[i].falloff());

    shader.set_uniform("point_lights[" + std::to_string(i) + "].cast_shadow",
                       point_lights[i].cast_shadow());

    if (point_lights[i].cast_shadow())
    {
      point_lights[i].shadow_tex()->bind_unit(global_texture_slot);
    }
    else
    {
      dummy_cube_texture_->bind_unit(global_texture_slot);
    }
    point_light_shadow_maps.push_back(global_texture_slot);
    ++global_texture_slot;
  }

  // bind rest of the cube samplers
  for (int i = point_light_count; i < max_point_light_count; ++i)
  {
    dummy_cube_texture_->bind_unit(global_texture_slot);
    point_light_shadow_maps.push_back(global_texture_slot);
    ++global_texture_slot;
  }
  shader.set_uniform("point_light_shadow_tex[0]", point_light_shadow_maps);

  const auto &directional_light = scene_render_info.directional_light();
  shader.set_uniform("smooth_shadows", smooth_shadows_);
  shader.set_uniform("shadow_bias_min", shadow_bias_min_);
  shader.set_uniform("light_size", light_size_);
  shader.set_uniform("directional_light_enabled", true);
  shader.set_uniform(
      "directional_light.direction",
      glm::vec3{view_matrix * glm::vec4{directional_light.direction(), 0.0f}});
  shader.set_uniform("directional_light.color", directional_light.color());
  shader.set_uniform("directional_light.multiplier",
                     directional_light.multiplier());

  shader.set_uniform("directional_light_shadow_enabled",
                     directional_light.cast_shadow());

  {
    shader.set_uniform("light_space_matrices[0]", light_space_matrices);

    shader.set_uniform("show_shadow_cascades", show_shadow_cascades_);
    std::vector<float> far_planes;
    std::transform(cascade_frustums.cbegin(),
                   cascade_frustums.cend(),
                   std::back_inserter(far_planes),
                   [](const auto &frustum) { return frustum.far; });
    shader.set_uniform("cascades_plane_distances[0]", far_planes);

    shadow_tex_array->bind_unit(global_texture_slot);
    shader.set_uniform("directional_light_shadow_tex", global_texture_slot);
    ++global_texture_slot;
  }
}

void ForwardPass::render_meshes(
    int                             &global_texture_slot,
    const EnvMapData                &env_map_data,
    const SceneRenderInfo           &scene_render_info,
    const ViewRenderInfo            &view_render_info,
    std::shared_ptr<GlTextureArray>  shadow_tex_array,
    const std::vector<glm::mat4>    &light_space_matrices,
    const std::vector<CascadeSplit> &cascade_frustums)
{
  const auto view_matrix = view_render_info.view_matrix();

  // prepare mesh pass
  mesh_shader_->bind();
  mesh_shader_->set_uniform("view_position", view_render_info.view_position());
  mesh_shader_->set_uniform("view_matrix", view_matrix);
  mesh_shader_->set_uniform("projection_matrix",
                            view_render_info.projection_matrix());

  set_ibl(*mesh_shader_, global_texture_slot, env_map_data);

  set_lightning(*mesh_shader_,
                global_texture_slot,
                scene_render_info,
                view_render_info,
                shadow_tex_array,
                light_space_matrices,
                cascade_frustums);

  // iterate through all meshes
  for (const auto &mesh : scene_render_info.meshes())
  {
    const auto material = mesh.mesh_->material();
    if (!material)
    {
      continue;
    }

    mesh_shader_->set_uniform("model_matrix", mesh.model_matrix_);

    int texture_slot = global_texture_slot;
    set_material(*mesh_shader_, texture_slot, *material);

    const auto vertex_array = mesh.mesh_->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }

  mesh_shader_->unbind();
}

void ForwardPass::render_skinned_meshes(
    int                             &global_texture_slot,
    const EnvMapData                &env_map_data,
    const SceneRenderInfo           &scene_render_info,
    const ViewRenderInfo            &view_render_info,
    std::shared_ptr<GlTextureArray>  shadow_tex_array,
    const std::vector<glm::mat4>    &light_space_matrices,
    const std::vector<CascadeSplit> &cascade_frustums)
{
  const auto &skinned_meshes = scene_render_info.skinned_meshes();
  if (skinned_meshes.empty())
  {
    return;
  }

  const auto view_matrix = view_render_info.view_matrix();

  // prepare mesh pass
  skinned_mesh_shader_->bind();
  skinned_mesh_shader_->set_uniform("view_position",
                                    view_render_info.view_position());
  skinned_mesh_shader_->set_uniform("view_matrix", view_matrix);
  skinned_mesh_shader_->set_uniform("projection_matrix",
                                    view_render_info.projection_matrix());

  set_ibl(*skinned_mesh_shader_, global_texture_slot, env_map_data);

  set_lightning(*skinned_mesh_shader_,
                global_texture_slot,
                scene_render_info,
                view_render_info,
                shadow_tex_array,
                light_space_matrices,
                cascade_frustums);

  // iterate through all meshes
  for (const auto &skinned_mesh : skinned_meshes)
  {
    const auto skinned_sub_mesh = skinned_mesh.skinned_sub_mesh_;
    const auto material         = skinned_sub_mesh->material();
    if (!material)
    {
      continue;
    }

    skinned_mesh_shader_->set_uniform("model_matrix",
                                      skinned_mesh.model_matrix_);
    skinned_mesh_shader_->set_uniform("bones[0]", skinned_mesh.bones_);

    int texture_slot = global_texture_slot;
    set_material(*skinned_mesh_shader_, texture_slot, *material);

    const auto vertex_array = skinned_sub_mesh->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }

  skinned_mesh_shader_->unbind();
}

void ForwardPass::render_meshes_depth_prepass(
    const SceneRenderInfo &scene_render_info,
    const ViewRenderInfo  &view_render_info)
{
  const auto view_matrix = view_render_info.view_matrix();

  // render depth pre pass
  depth_only_shader_->bind();
  depth_only_shader_->set_uniform("view_matrix", view_matrix);
  depth_only_shader_->set_uniform("projection_matrix",
                                  view_render_info.projection_matrix());
  for (const auto &mesh : scene_render_info.meshes())
  {
    const auto material = mesh.mesh_->material();
    if (!material)
    {
      continue;
    }

    depth_only_shader_->set_uniform("model_matrix", mesh.model_matrix_);

    const auto albedo_tex = material->albedo_texture();
    if (albedo_tex && albedo_tex->format() == GL_RGBA)
    {
      albedo_tex->bind_unit(1);
      depth_only_shader_->set_uniform("is_tex", true);
    }
    else
    {
      white_texture_->bind_unit(1);
      depth_only_shader_->set_uniform("is_tex", false);
    }
    depth_only_shader_->set_uniform("tex", 1);

    const auto vertex_array = mesh.mesh_->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }
  depth_only_shader_->unbind();
}

void ForwardPass::render_skinned_meshes_depth_prepass(
    const SceneRenderInfo &scene_render_info,
    const ViewRenderInfo  &view_render_info)
{
  // render depth pre pass for skinned meshes
  const auto skinned_meshes = scene_render_info.skinned_meshes();
  if (skinned_meshes.empty())
  {
    return;
  }

  const auto view_matrix = view_render_info.view_matrix();
  skinned_depth_only_shader_->bind();
  skinned_depth_only_shader_->set_uniform("view_matrix", view_matrix);
  skinned_depth_only_shader_->set_uniform("projection_matrix",
                                          view_render_info.projection_matrix());

  for (const auto &skinned_mesh : skinned_meshes)
  {
    const auto material = skinned_mesh.skinned_sub_mesh_->material();
    if (!material)
    {
      continue;
    }

    skinned_depth_only_shader_->set_uniform("model_matrix",
                                            skinned_mesh.model_matrix_);
    skinned_depth_only_shader_->set_uniform("bones[0]", skinned_mesh.bones_);

    const auto albedo_tex = material->albedo_texture();
    if (albedo_tex && albedo_tex->format() == GL_RGBA)
    {
      albedo_tex->bind_unit(1);
      skinned_depth_only_shader_->set_uniform("is_tex", true);
    }
    else
    {
      white_texture_->bind_unit(1);
      skinned_depth_only_shader_->set_uniform("is_tex", false);
    }
    skinned_depth_only_shader_->set_uniform("tex", 1);

    const auto vertex_array = skinned_mesh.skinned_sub_mesh_->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }
  skinned_depth_only_shader_->unbind();
}

void ForwardPass::set_material(GlShader       &shader,
                               int            &texture_slot,
                               const Material &material)
{
  shader.set_uniform("albedo_color", glm::vec3(1.0f));
  if (material.albedo_texture())
  {
    const auto albedo_texture = material.albedo_texture();
    albedo_texture->bind_unit(texture_slot);
    shader.set_uniform("albedo_tex", texture_slot);
    ++texture_slot;
  }
  else
  {
    white_texture_->bind_unit(texture_slot);
    shader.set_uniform("albedo_tex", texture_slot);
  }

  shader.set_uniform("roughness", 1.0f);
  shader.set_uniform("metalness", 1.0f);
  if (material.roughness_texture())
  {
    const auto roughness_texture = material.roughness_texture();
    roughness_texture->bind_unit(texture_slot);
    shader.set_uniform("metalness_roughness_tex", texture_slot);
    ++texture_slot;
  }
  else
  {
    white_texture_->bind_unit(texture_slot);
    shader.set_uniform("metalness_roughness_tex", texture_slot);
  }

  if (material.ambient_occlusion_texture())
  {
    const auto ao_texture = material.ambient_occlusion_texture();
    ao_texture->bind_unit(texture_slot);
    shader.set_uniform("ao_tex", texture_slot);
    shader.set_uniform("ao_tex_enabled", true);
    ++texture_slot;
  }
  else
  {
    white_texture_->bind_unit(texture_slot);
    shader.set_uniform("ao_tex", texture_slot);
    shader.set_uniform("ao_tex_enabled", false);
  }

  if (material.emissive_texture())
  {
    const auto emissive_texture = material.emissive_texture();
    emissive_texture->bind_unit(texture_slot);
    shader.set_uniform("emissive_tex", texture_slot);
    shader.set_uniform("emissive", glm::vec3(1.0f));
    ++texture_slot;
  }
  else
  {
    white_texture_->bind_unit(texture_slot);
    shader.set_uniform("emissive_tex", texture_slot);
    shader.set_uniform("emissive", glm::vec3(0.0f));
  }

  if (material.normal_texture())
  {
    const auto normal_texture = material.normal_texture();
    normal_texture->bind_unit(texture_slot);
    shader.set_uniform("normal_tex", texture_slot);
    shader.set_uniform("normal_tex_enabled", true);
    ++texture_slot;
  }
  else
  {
    white_texture_->bind_unit(texture_slot);
    shader.set_uniform("normal_tex", texture_slot);
    shader.set_uniform("normal_tex_enabled", false);
  }
}

void ForwardPass::execute(const SceneRenderInfo           &scene_render_info,
                          const ViewRenderInfo            &view_render_info,
                          std::shared_ptr<GlTextureArray>  shadow_tex_array,
                          const std::vector<glm::mat4>    &light_space_matrices,
                          const std::vector<CascadeSplit> &cascade_frustums)
{
  const auto &viewport_info = view_render_info.viewport_info();
  recreate_scene_framebuffer(viewport_info.width_, viewport_info.height_);

  scene_framebuffer_msaa_->bind();
  glCullFace(GL_BACK);

  int global_texture_slot{0};

  glViewport(0, 0, viewport_info.width_, viewport_info.height_);
  const glm::vec4 clear_color{0.0f, 0.0f, 0.0f, 1.0f};
  glClearNamedFramebufferfv(scene_framebuffer_msaa_->id(),
                            GL_COLOR,
                            0,
                            glm::value_ptr(clear_color));
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  const auto env_map_data = env_map(scene_render_info.env_map());

  if (!env_map_data.irradiance_tex_ || !env_map_data.prefilter_tex_)
  {
    // can not render anything without them
    if (output_)
    {
      output_(scene_render_info, view_render_info, scene_framebuffer_, nullptr);
    }
    return;
  }

  render_meshes_depth_prepass(scene_render_info, view_render_info);
  render_skinned_meshes_depth_prepass(scene_render_info, view_render_info);

  render_meshes(global_texture_slot,
                env_map_data,
                scene_render_info,
                view_render_info,
                shadow_tex_array,
                light_space_matrices,
                cascade_frustums);

  render_skinned_meshes(global_texture_slot,
                        env_map_data,
                        scene_render_info,
                        view_render_info,
                        shadow_tex_array,
                        light_space_matrices,
                        cascade_frustums);

  render_debug_lines(scene_render_info, view_render_info);

  scene_framebuffer_msaa_->unbind();

  // resolve msaa framebuffer info normal framebuffer
  glBlitNamedFramebuffer(scene_framebuffer_msaa_->id(),
                         scene_framebuffer_->id(),
                         0,
                         0,
                         scene_framebuffer_width_,
                         scene_framebuffer_height_,
                         0,
                         0,
                         scene_framebuffer_width_,
                         scene_framebuffer_height_,
                         GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                         GL_NEAREST);

  if (output_)
  {
    output_(scene_render_info,
            view_render_info,
            scene_framebuffer_,
            env_map_data.irradiance_tex_);
  }
}

void ForwardPass::init_shaders()
{
  line_shader_ = std::make_shared<GlShader>();
  line_shader_->init("shaders/line.vert", "shaders/line.frag");

  depth_only_shader_ = std::make_shared<GlShader>();
  depth_only_shader_->init("shaders/pbr.vert", "shaders/depth_only.frag");

  skinned_depth_only_shader_ = std::make_shared<GlShader>();
  skinned_depth_only_shader_->init("shaders/pbr.vert",
                                   "shaders/depth_only.frag",
                                   std::vector<std::string>{"SKINNED 1"});

  mesh_shader_ = std::make_shared<GlShader>();
  mesh_shader_->init("shaders/pbr.vert", "shaders/pbr.frag");

  skinned_mesh_shader_ = std::make_shared<GlShader>();
  skinned_mesh_shader_->init("shaders/pbr.vert",
                             "shaders/pbr.frag",
                             std::vector<std::string>{"SKINNED 1"});

  equirectangular_to_cubemap_shader_ = std::make_shared<GlShader>();
  equirectangular_to_cubemap_shader_->init(
      "shaders/learnopengl/cubemap.vert",
      "shaders/learnopengl/equirectangular_to_cubemap.frag");

  irradiance_shader_ = std::make_shared<GlShader>();
  irradiance_shader_->init("shaders/learnopengl/cubemap.vert",
                           "shaders/learnopengl/irradiance_convolution.frag");

  prefilter_shader_ = std::make_shared<GlShader>();
  prefilter_shader_->init("shaders/learnopengl/cubemap.vert",
                          "shaders/learnopengl/prefilter.frag");
}

void ForwardPass::recreate_scene_framebuffer(int width, int height)
{
  if (width == scene_framebuffer_width_ && height == scene_framebuffer_height_)
  {
    return;
  }
  scene_framebuffer_width_  = width;
  scene_framebuffer_height_ = height;

  {
    static constexpr GLuint samples{8};
    // create msaa framebuffer
    FramebufferAttachmentCreateConfig color_config{};
    color_config.type_            = AttachmentType::Texture;
    color_config.format_          = GL_RGBA;
    color_config.internal_format_ = GL_RGBA16F;
    color_config.width_           = scene_framebuffer_width_;
    color_config.height_          = scene_framebuffer_height_;
    color_config.msaa_            = samples;

    FramebufferAttachmentCreateConfig depth_config{};
    depth_config.type_            = AttachmentType::Renderbuffer;
    depth_config.format_          = GL_DEPTH_COMPONENT;
    depth_config.internal_format_ = GL_DEPTH_COMPONENT32F;
    depth_config.width_           = scene_framebuffer_width_;
    depth_config.height_          = scene_framebuffer_height_;
    depth_config.msaa_            = samples;

    FramebufferConfig config{};
    config.color_attachments_.push_back(color_config);
    config.depth_attachment_ = depth_config;

    scene_framebuffer_msaa_ = std::make_shared<GlFramebuffer>();
    scene_framebuffer_msaa_->attach(config);
  }

  {
    // create framebuffer to resolve msaa buffer
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
}

void ForwardPass::set_output(Output output) { output_ = output; }

ForwardPass::EnvMapData
ForwardPass::generate_env_map(const EnvironmentMap &env_map)
{
  FramebufferAttachmentCreateConfig depth_attachment_config{};
  depth_attachment_config.type_            = AttachmentType::Renderbuffer;
  depth_attachment_config.format_          = GL_DEPTH_COMPONENT;
  depth_attachment_config.internal_format_ = GL_DEPTH_COMPONENT24;
  depth_attachment_config.height_          = 512;
  depth_attachment_config.width_           = 512;

  FramebufferConfig framebuffer_config{};
  framebuffer_config.depth_attachment_ = depth_attachment_config;

  const auto capture_framebuffer = std::make_shared<GlFramebuffer>();
  capture_framebuffer->attach(framebuffer_config);

  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  capture_framebuffer->bind();

  const auto &hdr_data = env_map.data();

  stbi_set_flip_vertically_on_load(true);
  int  width, height, nrComponents;
  auto data = stbi_loadf_from_memory(hdr_data.data(),
                                     hdr_data.size(),
                                     &width,
                                     &height,
                                     &nrComponents,
                                     0);

  std::shared_ptr<GlTexture> hdr_texture{};
  if (data)
  {
    GlTextureConfig texture_config{};
    texture_config.data_             = data;
    texture_config.width_            = width;
    texture_config.height_           = height;
    texture_config.format_           = GL_RGB;
    texture_config.sized_format_     = GL_RGB16F;
    texture_config.wrap_s_           = GL_CLAMP_TO_EDGE;
    texture_config.wrap_t_           = GL_CLAMP_TO_EDGE;
    texture_config.min_filter_       = GL_LINEAR;
    texture_config.mag_filter_       = GL_LINEAR;
    texture_config.generate_mipmaps_ = false;
    texture_config.type_             = GL_FLOAT;

    hdr_texture = std::make_shared<GlTexture>(texture_config);

    stbi_image_free(data);
  }
  else
  {
    throw std::runtime_error("Failed to load HDR image.");
  }

  // setup cubemap to render to and attach to framebuffer
  GlCubeTextureConfig env_cubemap_config{};
  env_cubemap_config.width_  = 512;
  env_cubemap_config.height_ = 512;
  const auto env_cubemap = std::make_shared<GlCubeTexture>(env_cubemap_config);

  // set up projection and view matrices for capturing data onto the 6
  // cubemap face directions
  glm::mat4 captureProjection =
      glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 captureViews[] = {glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(1.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, -1.0f, 0.0f)),
                              glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(-1.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, -1.0f, 0.0f)),
                              glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f),
                                          glm::vec3(0.0f, 0.0f, 1.0f)),
                              glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, -1.0f, 0.0f),
                                          glm::vec3(0.0f, 0.0f, -1.0f)),
                              glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 0.0f, 1.0f),
                                          glm::vec3(0.0f, -1.0f, 0.0f)),
                              glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 0.0f, -1.0f),
                                          glm::vec3(0.0f, -1.0f, 0.0f))};

  // convert HDR equirectangular environment map to cubemap equivalent
  equirectangular_to_cubemap_shader_->bind();
  equirectangular_to_cubemap_shader_->set_uniform("equirectangularMap", 0);
  equirectangular_to_cubemap_shader_->set_uniform("projection",
                                                  captureProjection);
  hdr_texture->bind_unit(0);

  // don't forget to configure the viewport to the
  // capture dimensions.
  glViewport(0, 0, 512, 512);
  capture_framebuffer->bind();
  for (unsigned int i = 0; i < 6; ++i)
  {
    equirectangular_to_cubemap_shader_->set_uniform("view", captureViews[i]);
    capture_framebuffer->set_color_attachment(0, env_cubemap, i);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCube();
  }
  capture_framebuffer->unbind();

  // then let OpenGL generate mipmaps from first mip face (combatting
  // visible dots artifact)
  glGenerateTextureMipmap(env_cubemap->id());

  // create an irradiance cubemap, and re-scale capture FBO to irradiance
  // scale.
  GlCubeTextureConfig irr_cubemap_config{};
  irr_cubemap_config.width_            = 32;
  irr_cubemap_config.height_           = 32;
  irr_cubemap_config.generate_mipmaps_ = false;
  irr_cubemap_config.min_filter_       = GL_LINEAR;
  irr_cubemap_config.mag_filter_       = GL_LINEAR;
  const auto irradiance_map =
      std::make_shared<GlCubeTexture>(irr_cubemap_config);

  capture_framebuffer->bind();
  GlRenderbufferConfig renderbuffer_config{};
  renderbuffer_config.width_        = 32;
  renderbuffer_config.height_       = 32;
  renderbuffer_config.sized_format_ = GL_DEPTH_COMPONENT24;
  auto renderbuffer = std::make_shared<GlRenderbuffer>(renderbuffer_config);
  capture_framebuffer->set_depth_attachment(renderbuffer);

  // solve diffuse integral by convolution to create an irradiance
  // (cube)map.
  irradiance_shader_->bind();
  irradiance_shader_->set_uniform("environmentMap", 0);
  irradiance_shader_->set_uniform("projection", captureProjection);
  env_cubemap->bind_unit(0);

  // don't forget to configure the viewport to the
  // capture dimensions.
  glViewport(0, 0, 32, 32);
  capture_framebuffer->bind();

  for (unsigned int i = 0; i < 6; ++i)
  {
    irradiance_shader_->set_uniform("view", captureViews[i]);
    capture_framebuffer->set_color_attachment(0, irradiance_map, i);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCube();
  }
  capture_framebuffer->unbind();

  // create a pre-filter cubemap, and re-scale capture FBO to
  // pre-filter scale.
  GlCubeTextureConfig prefilter_map_config{};
  prefilter_map_config.width_            = 128;
  prefilter_map_config.height_           = 128;
  prefilter_map_config.generate_mipmaps_ = true;
  prefilter_map_config.min_filter_       = GL_LINEAR_MIPMAP_LINEAR;
  prefilter_map_config.mag_filter_       = GL_LINEAR;
  const auto prefilter_map =
      std::make_shared<GlCubeTexture>(prefilter_map_config);

  // run a quasi monte-carlo simulation on the environment lighting to
  // create a prefilter (cube)map.
  prefilter_shader_->bind();
  prefilter_shader_->set_uniform("environmentMap", 0);
  prefilter_shader_->set_uniform("projection", captureProjection);
  env_cubemap->bind_unit(0);

  capture_framebuffer->bind();

  const auto max_mip_levels = prefilter_map->mipmap_levels();
  for (unsigned int mip = 0; mip < prefilter_map->mipmap_levels(); ++mip)
  {
    // reisze framebuffer according to mip-level size.
    const auto mip_width  = static_cast<unsigned int>(128 * std::pow(0.5, mip));
    const auto mip_height = static_cast<unsigned int>(128 * std::pow(0.5, mip));
    GlRenderbufferConfig renderbuffer_config{};
    renderbuffer_config.width_        = mip_width;
    renderbuffer_config.height_       = mip_height;
    renderbuffer_config.sized_format_ = GL_DEPTH_COMPONENT24;
    auto renderbuffer = std::make_shared<GlRenderbuffer>(renderbuffer_config);
    capture_framebuffer->set_depth_attachment(renderbuffer);

    glViewport(0, 0, mip_width, mip_height);

    const auto roughness =
        static_cast<float>(mip) / static_cast<float>(max_mip_levels - 1);
    prefilter_shader_->set_uniform("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i)
    {
      prefilter_shader_->set_uniform("view", captureViews[i]);
      capture_framebuffer->set_color_attachment(0, prefilter_map, i, mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderCube();
    }
  }
  capture_framebuffer->unbind();
  glEnable(GL_CULL_FACE);

  EnvMapData env_map_data{};
  env_map_data.irradiance_tex_ = irradiance_map;
  env_map_data.prefilter_tex_  = prefilter_map;

  return env_map_data;
}

ForwardPass::EnvMapData ForwardPass::env_map(const EnvironmentMap &env_map)
{
  const auto name = env_map.name();
  const auto iter = env_maps_.find(name);
  if (iter == env_maps_.end())
  {
    try
    {
      const auto env_map_data = generate_env_map(env_map);
      env_maps_[name]         = env_map_data;
      return env_map_data;
    }
    catch (const std::runtime_error &error)
    {
      return {};
    }
  }

  return iter->second;
}

void ForwardPass::render_debug_lines(const SceneRenderInfo &scene_render_info,
                                     const ViewRenderInfo  &view_render_info)
{
  const auto &debug_lines = scene_render_info.debug_lines();
  if (debug_lines.empty())
  {
    return;
  }

  line_shader_->bind();

  line_shader_->set_uniform("projection_matrix",
                            view_render_info.projection_matrix());
  line_shader_->set_uniform("view_matrix", view_render_info.view_matrix());

  long debug_lines_count = debug_lines.size();
  long debug_lines_index{0};

  while (debug_lines_count > 0)
  {
    const auto count =
        std::min((static_cast<long>(debug_lines.size()) - debug_lines_index),
                 static_cast<long>(max_debug_lines_count));

    lines_vertex_buffer_->write(debug_lines.data() + debug_lines_index,
                                count * sizeof(DebugLineInfo),
                                0);
    draw(*lines_vertex_array_, GL_LINES, count * 2);

    debug_lines_count -= count;
    debug_lines_index += count;
  }

  line_shader_->unbind();
}

} // namespace dc
