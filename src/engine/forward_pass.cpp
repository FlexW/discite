#include "forward_pass.hpp"
#include "gl_shader.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"
#include "log.hpp"
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

void ForwardPass::execute(const SceneRenderInfo &         scene_render_info,
                          const ViewRenderInfo &          view_render_info,
                          std::shared_ptr<GlTextureArray> shadow_tex_array,
                          const std::vector<glm::mat4>    light_space_matrices,
                          const std::vector<CascadeSplit> cascade_frustums)
{
  static unsigned int hdrTexture{};
  static unsigned int envCubemap{};
  static unsigned int prefilterMap{};
  static unsigned int captureFBO{};
  static unsigned int captureRBO{};

  if (need_generate_prefilter_map_)
  {
    glDisable(GL_CULL_FACE);
    need_generate_prefilter_map_ = false;

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter
    // map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              captureRBO);

    stbi_set_flip_vertically_on_load(true);
    int    width, height, nrComponents;
    float *data = stbi_loadf("game_data/src/piazza_bologni.hdr",
                             &width,
                             &height,
                             &nrComponents,
                             0);
    if (data)
    {
      glGenTextures(1, &hdrTexture);
      glBindTexture(GL_TEXTURE_2D, hdrTexture);
      glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGB16F,
          width,
          height,
          0,
          GL_RGB,
          GL_FLOAT,
          data); // note how we specify the texture's data value to be float

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
    }
    else
    {
      throw std::runtime_error("Failed to load HDR image.");
    }

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0,
                   GL_RGB16F,
                   512,
                   512,
                   0,
                   GL_RGB,
                   GL_FLOAT,
                   nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling
                                  // (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6
    // cubemap face directions
    // ----------------------------------------------------------------------------------------------
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

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    equirectangular_to_cubemap_shader_->bind();
    equirectangular_to_cubemap_shader_->set_uniform("equirectangularMap", 0);
    equirectangular_to_cubemap_shader_->set_uniform("projection",
                                                    captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the
                                // capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
      equirectangular_to_cubemap_shader_->set_uniform("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             envCubemap,
                             0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting
    // visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: create a pre-filter cubemap, and re-scale capture FBO to
    // pre-filter scale.
    // --------------------------------------------------------------------------------
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0,
                   GL_RGB16F,
                   128,
                   128,
                   0,
                   GL_RGB,
                   GL_FLOAT,
                   nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification
                                              // filter to mip_linear
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the
    // required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: run a quasi monte-carlo simulation on the environment lighting to
    // create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilter_shader_->bind();
    prefilter_shader_->set_uniform("environmentMap", 0);
    prefilter_shader_->set_uniform("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    unsigned int maxMipLevels = math::calc_mipmap_levels_2d(128, 128);
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
      // reisze framebuffer according to mip-level size.
      unsigned int mipWidth =
          static_cast<unsigned int>(128 * std::pow(0.5, mip));
      unsigned int mipHeight =
          static_cast<unsigned int>(128 * std::pow(0.5, mip));
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER,
                            GL_DEPTH_COMPONENT24,
                            mipWidth,
                            mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);

      float roughness = (float)mip / (float)(maxMipLevels - 1);
      prefilter_shader_->set_uniform("roughness", roughness);
      for (unsigned int i = 0; i < 6; ++i)
      {
        prefilter_shader_->set_uniform("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               prefilterMap,
                               mip);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
      }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE);
  }

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

  const auto &sky = scene_render_info.env_map();
  if (!sky.env_texture() || !sky.env_irradiance_texture())
  {
    // can not render anything without them
    if (output_)
    {
      output_(scene_render_info, view_render_info, scene_framebuffer_);
    }
    return;
  }

  const auto view_matrix = view_render_info.view_matrix();

  // render depth pre pass
  depth_only_shader_->bind();
  depth_only_shader_->set_uniform("view_matrix", view_matrix);
  depth_only_shader_->set_uniform("projection_matrix",
                                  view_render_info.projection_matrix());
  for (const auto &mesh : scene_render_info.meshes())
  {
    depth_only_shader_->set_uniform("model_matrix", mesh.model_matrix_);

    const auto albedo_tex = mesh.mesh_->material()->albedo_texture();
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

  mesh_shader_->bind();
  mesh_shader_->set_uniform("view_matrix", view_matrix);
  mesh_shader_->set_uniform("projection_matrix",
                            view_render_info.projection_matrix());

  brdf_lut_texture_->bind_unit(global_texture_slot);
  mesh_shader_->set_uniform("brdf_lut_tex", global_texture_slot);
  ++global_texture_slot;

  glBindTextureUnit(global_texture_slot, prefilterMap);
  // sky.env_texture()->bind_unit(global_texture_slot);
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
    mesh_shader_->set_uniform(
        "point_lights[" + std::to_string(i) + "].position",
        glm::vec3(view_matrix * glm::vec4(point_lights[i].position(), 1.0f)));

    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) + "].color",
                              point_lights[i].color());
    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                  "].multiplier",
                              point_lights[i].multiplier());

    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) + "].radius",
                              point_lights[i].radius());
    mesh_shader_->set_uniform("point_lights[" + std::to_string(i) + "].falloff",
                              point_lights[i].falloff());
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
  mesh_shader_->set_uniform("directional_light.multiplier",
                            directional_light.multiplier());

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
      mesh_shader_->set_uniform("albedo_tex", texture_slot);
      mesh_shader_->set_uniform("albedo_color", glm::vec3(1.0f));
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("in_albedo_tex", texture_slot);
    }

    if (material->roughness_texture())
    {
      const auto roughness_texture = material->roughness_texture();
      roughness_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("metalness_roughness_tex", texture_slot);
      mesh_shader_->set_uniform("roughness", 1.0f);
      mesh_shader_->set_uniform("metalness", 1.0f);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("metalness_roughness_tex", texture_slot);
    }

    if (material->ambient_occlusion_texture())
    {
      const auto ao_texture = material->ambient_occlusion_texture();
      ao_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("ao_tex", texture_slot);
      mesh_shader_->set_uniform("ao_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("ao_tex", texture_slot);
      mesh_shader_->set_uniform("ao_tex_enabled", false);
    }

    if (material->emissive_texture())
    {
      const auto emissive_texture = material->emissive_texture();
      emissive_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("emissive_tex", texture_slot);
      mesh_shader_->set_uniform("emissive", glm::vec3(1.0f));
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("emissive_tex", texture_slot);
      mesh_shader_->set_uniform("emissive", glm::vec3(0.0f));
    }

    if (material->normal_texture())
    {
      const auto normal_texture = material->normal_texture();
      normal_texture->bind_unit(texture_slot);
      mesh_shader_->set_uniform("normal_tex", texture_slot);
      mesh_shader_->set_uniform("normal_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      white_texture_->bind_unit(texture_slot);
      mesh_shader_->set_uniform("normal_tex", texture_slot);
      mesh_shader_->set_uniform("normal_tex_enabled", false);
    }

    const auto vertex_array = mesh.mesh_->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }

  mesh_shader_->unbind();
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
    output_(scene_render_info, view_render_info, scene_framebuffer_);
  }
}

void ForwardPass::init_shaders()
{
  depth_only_shader_ = std::make_shared<GlShader>();
  depth_only_shader_->init("shaders/pbr.vert", "shaders/depth_only.frag");

  mesh_shader_ = std::make_shared<GlShader>();
  mesh_shader_->init("shaders/pbr.vert", "shaders/pbr.frag");

  equirectangular_to_cubemap_shader_ = std::make_shared<GlShader>();
  equirectangular_to_cubemap_shader_->init(
      "shaders/learnopengl/cubemap.vert",
      "shaders/learnopengl/equirectangular_to_cubemap.frag");

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

} // namespace dc
