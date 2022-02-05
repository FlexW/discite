#include "blinn_phong_application.hpp"
#include "engine/application.hpp"
#include "engine/defer.hpp"
#include "engine/gl_framebuffer.hpp"
#include "engine/gl_shader.hpp"
#include "engine/gl_texture.hpp"
#include "engine/gl_texture_array.hpp"
#include "engine/imgui.hpp"
#include "engine/math.hpp"
#include "engine/mesh.hpp"
#include "engine/point_light.hpp"
#include "imgui.h"

#include <cstddef>
#include <functional>
#include <ios>
#include <memory>
#include <string>
#include <vector>

BlinnPhongApplication::~BlinnPhongApplication()
{
  if (quad_vertex_array_)
  {
    glDeleteVertexArrays(1, &quad_vertex_array_);
  }
}

void BlinnPhongApplication::init()
{
  Application::init();

  calc_shadow_cascades_splits();

  // enable the cursor
  set_move_camera(false);

  // get the window size
  glfwGetFramebufferSize(glfw_window(), &window_width_, &window_height_);

  // load cubes
  // texture_cache_.set_import_path("data/cubes");
  // auto cubes_model = std::make_shared<Model>();
  // cubes_model->load_from_file("data/cubes/cubes.gltf", texture_cache_);
  // add_model(cubes_model);

  // load sponza
  texture_cache_.set_import_path(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF");
  auto sponza_model = std::make_shared<Model>();
  sponza_model->load_from_file(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
      texture_cache_);
  add_model(sponza_model);

  PointLight point_light1{};
  point_light1.set_position(glm::vec3{-9.5f, 1.0f, 3.0f});
  point_light1.set_ambient_color(glm::vec3{100.0f, 38.0f, 8.6f});
  point_light1.set_diffuse_color(glm::vec3{100.0f, 38.0f, 8.6f});
  point_light1.set_specular_color(glm::vec3{100.0f, 38.0f, 8.6f});
  point_lights_.push_back(point_light1);

  PointLight point_light2{};
  point_light2.set_position(glm::vec3{-9.5f, 1.0f, -3.0f});
  point_light2.set_ambient_color(glm::vec3{83.0f, 27.0f, 0.0f});
  point_light2.set_diffuse_color(glm::vec3{83.0f, 27.0f, 0.0f});
  point_light2.set_specular_color(glm::vec3{83.0f, 27.0f, 0.0f});
  point_lights_.push_back(point_light2);

  PointLight point_light3{};
  point_light3.set_position(glm::vec3{9.5f, 1.0f, 3.0f});
  point_light3.set_ambient_color(glm::vec3{83.0f, 27.0f, 0.0f});
  point_light3.set_diffuse_color(glm::vec3{83.0f, 27.0f, 0.0f});
  point_light3.set_specular_color(glm::vec3{83.0f, 27.0f, 0.0f});
  point_lights_.push_back(point_light3);

  PointLight point_light4{};
  point_light4.set_position(glm::vec3{9.5f, 1.0f, -3.0f});
  point_light4.set_ambient_color(glm::vec3{57.0f, 18.0f, 0.0f});
  point_light4.set_diffuse_color(glm::vec3{57.0f, 18.0f, 0.0f});
  point_light4.set_specular_color(glm::vec3{57.0f, 18.0f, 0.0f});
  point_lights_.push_back(point_light4);

  // load runholt
  // texture_cache_.set_import_path("external/deps/src/Rungholt");
  // auto rungholt_model = std::make_shared<Model>();
  // rungholt_model->load_from_file("external/deps/src/Rungholt/rungholt.obj",
  //                                texture_cache_);
  // add_model(rungholt_model);

  // load bistro
  // texture_cache_.set_import_path("external/deps/src/bistro/Exterior");
  // auto bistro_model = std::make_shared<Model>();
  // bistro_model->load_from_file("external/deps/src/bistro/Exterior/exterior.obj",
  //                              texture_cache_);
  // bistro_model->set_scale(glm::vec3{0.01f});
  // models_.push_back(bistro_model);

  // reset texture cache path
  texture_cache_.set_import_path("data/textures");

  // load model shader
  model_shader_ = std::make_shared<GlShader>();
  model_shader_->init("shaders/blinn_phong.vert", "shaders/blinn_phong.frag");

  recalculate_projection_matrix();

  // create scene framebuffer
  recreate_scene_framebuffer();

  // create framebuffer for shadow map
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

  // load shadow map shaders
  shadow_map_shader_ = std::make_shared<GlShader>();
  shadow_map_shader_->init("shaders/shadow_map.vert",
                           "shaders/shadow_map.geom",
                           "shaders/shadow_map.frag");

  shadow_map_transparent_shader_ = std::make_shared<GlShader>();
  shadow_map_transparent_shader_->init("shaders/shadow_map_transparent.vert",
                                       "shaders/shadow_map_transparent.geom",
                                       "shaders/shadow_map_transparent.frag");

  // load depth debug shader
  glGenVertexArrays(1, &quad_vertex_array_);
  depth_debug_shader_ = std::make_shared<GlShader>();
  depth_debug_shader_->init("shaders/quad.vert", "shaders/debug_depth.frag");

  // load hdr shader
  hdr_shader_ = std::make_shared<GlShader>();
  hdr_shader_->init("shaders/quad.vert", "shaders/hdr.frag");

  // setup sun
  directional_light_.set_direction(glm::vec3{-0.49f, -0.64f, -0.59f});
  directional_light_.set_ambient_color(glm::vec3{1.0f});
  directional_light_.set_diffuse_color(glm::vec3{15.0f});
  directional_light_.set_specular_color(glm::vec3{50.0f});

  white_texture_ = std::make_shared<GlTexture>();
  std::vector<unsigned char> white_tex_data{255};
  white_texture_->set_data(white_tex_data.data(), 1, 1, 1, false);
}

void BlinnPhongApplication::on_window_framebuffer_size_callback(
    GLFWwindow * /*window*/,
    int width,
    int height)
{
  window_width_  = width;
  window_height_ = height;

  recalculate_projection_matrix();
  recreate_scene_framebuffer();
}

void BlinnPhongApplication::on_window_close_callback(GLFWwindow *window)
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void BlinnPhongApplication::on_mouse_movement_callback(GLFWwindow * /*window*/,
                                                       double x,
                                                       double y)
{
  auto x_offset = 0.0;
  auto y_offset = 0.0;

  if (is_mouse_first_move_)
  {
    mouse_last_x_ = x;
    mouse_last_y_ = y;

    is_mouse_first_move_ = false;
  }
  else
  {
    x_offset = x - mouse_last_x_;
    y_offset = mouse_last_y_ - y;

    mouse_last_x_ = x;
    mouse_last_y_ = y;
  }

  if (is_move_camera_)
  {
    camera_.process_rotation(x_offset, y_offset);
  }
}

std::vector<glm::vec4> BlinnPhongApplication::calc_frustum_corners(
    const glm::mat4 &projection_matrix) const
{
  const auto inv = glm::inverse(projection_matrix * camera_.view_matrix());

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

glm::mat4 BlinnPhongApplication::calc_light_space_matrix(float near_plane,
                                                         float far_plane) const
{
  const auto projection = glm::perspective(
      glm::radians(camera_.zoom()),
      static_cast<float>(window_width_) / static_cast<float>(window_height_),
      near_plane,
      far_plane);
  const auto frustum_corners = calc_frustum_corners(projection);

  glm::vec3 center{0.0f};
  for (const auto &corner : frustum_corners)
  {
    center += glm::vec3{corner};
  }
  center /= frustum_corners.size();

  const auto light_view = glm::lookAt(center - directional_light_.direction(),
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

std::vector<glm::mat4> BlinnPhongApplication::calc_light_space_matrices() const
{
  std::vector<glm::mat4> matrices;
  matrices.reserve(cascade_frustums_.size());
  for (const auto &frustum : cascade_frustums_)
  {
    matrices.push_back(calc_light_space_matrix(frustum.near, frustum.far));
  }

  // for (std::size_t i = 0; i < shadow_cascades_levels_.size() + 1; ++i)
  // {
  //   if (i == 0)
  //   {
  //     matrices.push_back(
  //         calc_light_space_matrix(camera_near_, shadow_cascades_levels_[i]));
  //   }
  //   else if (i < shadow_cascades_levels_.size())
  //   {
  //     matrices.push_back(calc_light_space_matrix(shadow_cascades_levels_[i -
  //     1],
  //                                                shadow_cascades_levels_[i]));
  //   }
  //   else
  //   {
  //     matrices.push_back(
  //         calc_light_space_matrix(shadow_cascades_levels_[i - 1],
  //         camera_far_));
  //   }
  // }

  return matrices;
}

void BlinnPhongApplication::on_update(float delta_time)
{
  // do we need to close the application?
  if (key(GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    close();
    return;
  }

  // move the camera
  move_camera(delta_time);

  // sort in transparent and solid meshes
  const auto estimated_meshes_count = models_.size() * 10;
  solid_meshes_.reserve(estimated_meshes_count);
  transparent_meshes_.reserve(estimated_meshes_count);
  defer(solid_meshes_.clear());
  defer(transparent_meshes_.clear());

  for (const auto &model : models_)
  {
    for (const auto &mesh : model->meshes())
    {
      MeshInfo mesh_info{};
      mesh_info.mesh         = mesh;
      mesh_info.model_matrix = model->model_matrix();

      if (mesh->material()->is_transparent())
      {
        transparent_meshes_.push_back(mesh_info);
      }
      else
      {
        solid_meshes_.push_back(mesh_info);
      }
    }
  }

  const auto light_space_matrices = calc_light_space_matrices();

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
      shadow_map_shader_->set_uniform("model_matrix", mesh_info.model_matrix);

      draw(*mesh_info.mesh->vertex_array(), GL_TRIANGLES);
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
      const auto diffuse_tex = mesh_info.mesh->material()->diffuse_texture();
      if (!diffuse_tex)
      {
        continue;
      }
      // TODO: why does 1 work and 0 not?
      glActiveTexture(GL_TEXTURE1);
      diffuse_tex->bind();
      shadow_map_transparent_shader_->set_uniform("tex", 1);
      shadow_map_transparent_shader_->set_uniform("model_matrix",
                                                  mesh_info.model_matrix);

      draw(*mesh_info.mesh->vertex_array(), GL_TRIANGLES);
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

    glViewport(0, 0, window_width_, window_height_);
    glClearColor(sky_color_.r, sky_color_.g, sky_color_.b, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    const auto view_matrix = camera_.view_matrix();

    model_shader_->bind();
    model_shader_->set_uniform("view_matrix", view_matrix);
    model_shader_->set_uniform("projection_matrix", projection_matrix_);

    const int point_light_count =
        point_lights_.size() <= 5 ? point_lights_.size() : 5;
    model_shader_->set_uniform("point_light_count", point_light_count);
    for (int i = 0; i < point_light_count; ++i)
    {
      model_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                     "].ambient_color",
                                 point_lights_[i].ambient_color());
      model_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                     "].diffuse_color",
                                 point_lights_[i].diffuse_color());
      model_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                     "].specular_color",
                                 point_lights_[i].specular_color());
      model_shader_->set_uniform(
          "point_lights[" + std::to_string(i) + "].position",
          glm::vec3(view_matrix *
                    glm::vec4(point_lights_[i].position(), 1.0f)));
      model_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                     "].linear",
                                 point_lights_[i].linear());
      model_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                     "].constant",
                                 point_lights_[i].constant());
      model_shader_->set_uniform("point_lights[" + std::to_string(i) +
                                     "].quadratic",
                                 point_lights_[i].quadratic());
    }

    model_shader_->set_uniform("spot_light_count", 0);

    model_shader_->set_uniform("smooth_shadows", smooth_shadows_);
    model_shader_->set_uniform("shadow_bias_min", shadow_bias_min_);
    model_shader_->set_uniform("light_size", light_size_);
    model_shader_->set_uniform("directional_light_enabled", true);
    model_shader_->set_uniform("directional_light.direction_ws",
                               directional_light_.direction());
    model_shader_->set_uniform(
        "directional_light.direction",
        glm::vec3{view_matrix *
                  glm::vec4{directional_light_.direction(), 0.0f}});
    model_shader_->set_uniform("directional_light.ambient_color",
                               directional_light_.ambient_color());
    model_shader_->set_uniform("directional_light.diffuse_color",
                               directional_light_.diffuse_color());
    model_shader_->set_uniform("directional_light.specular_color",
                               directional_light_.specular_color());

    model_shader_->set_uniform("directional_light_shadow_enabled", is_shadows_enabled_);
    model_shader_->set_uniform("far_plane", camera_far_);

    {
      for (std::size_t i = 0; i < light_space_matrices.size(); ++i)
      {
        model_shader_->set_uniform("light_space_matrices[" + std::to_string(i) +
                                       "]",
                                   light_space_matrices[i]);
      }

      model_shader_->set_uniform("show_shadow_cascades", show_shadow_cascades_);
      for (std::size_t i = 0; i < shadow_cascades_count_; ++i)
      {
        model_shader_->set_uniform("cascades_plane_distances[" +
                                       std::to_string(i) + "]",
                                   cascade_frustums_[i].far);
      }

      glActiveTexture(GL_TEXTURE0 + global_texture_slot);
      shadow_tex_array_->bind();
      model_shader_->set_uniform("directional_light_shadow_tex",
                                 global_texture_slot);
      ++global_texture_slot;
    }

    // iterate through all models
    for (const auto &model : models_)
    {
      model_shader_->set_uniform("model_matrix", model->model_matrix());

      // iterate through all meshes
      const auto &meshes = model->meshes();
      for (const auto &mesh : meshes)
      {
        int texture_slot = global_texture_slot;

        const auto material = mesh->material();
        if (material->diffuse_texture())
        {
          const auto diffuse_texture = material->diffuse_texture();
          glActiveTexture(GL_TEXTURE0 + texture_slot);
          diffuse_texture->bind();
          model_shader_->set_uniform("in_diffuse_tex", texture_slot);
          model_shader_->set_uniform("diffuse_tex_enabled", true);
          ++texture_slot;
        }
        else
        {
          glActiveTexture(GL_TEXTURE0 + texture_slot);
          white_texture_->bind();
          model_shader_->set_uniform("in_diffuse_tex", texture_slot);
          model_shader_->set_uniform("in_diffuse_color",
                                     material->diffuse_color());
          model_shader_->set_uniform("diffuse_tex_enabled", false);
        }

        if (material->normal_texture())
        {
          const auto normal_texture = material->normal_texture();
          glActiveTexture(GL_TEXTURE0 + texture_slot);
          normal_texture->bind();
          model_shader_->set_uniform("in_normal_tex", texture_slot);
          model_shader_->set_uniform("normal_tex_enabled", true);
          ++texture_slot;
        }
        else
        {
          glActiveTexture(GL_TEXTURE0 + texture_slot);
          white_texture_->bind();
          model_shader_->set_uniform("in_normal_tex", texture_slot);
          model_shader_->set_uniform("normal_tex_enabled", false);
        }

        model_shader_->set_uniform("specular_power",
                                   material->specular_power());

        const auto vertex_array = mesh->vertex_array();
        draw(*vertex_array, GL_TRIANGLES);
      }
    }

    model_shader_->unbind();
    scene_framebuffer_->unbind();
  }

  // perform hdr to sdr conversation
  {
    // make sure the framebuffer of the window gets used
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width_, window_height_);
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
  }
}

void BlinnPhongApplication::recalculate_projection_matrix()
{
  projection_matrix_ =
      glm::perspective(glm::radians(camera_.zoom()),
                       static_cast<float>(window_width_) / window_height_,
                       camera_near_,
                       camera_far_);
}

void BlinnPhongApplication::add_model(std::shared_ptr<Model> model)
{
  models_.push_back(model);
}

void BlinnPhongApplication::render_imgui_general()
{
  {
    ImGui::Text("Sky");
    imgui_input("Color", sky_color_);
  }
  ImGui::Separator();
  {
    ImGui::Text("Sun");

    auto direction = directional_light_.direction();
    imgui_input("Direction", direction);
    directional_light_.set_direction(direction);

    auto ambient_color = directional_light_.ambient_color();
    imgui_input("Ambient color", ambient_color);
    directional_light_.set_ambient_color(ambient_color);

    auto diffuse_color = directional_light_.diffuse_color();
    imgui_input("Diffuse color", diffuse_color);
    directional_light_.set_diffuse_color(diffuse_color);

    auto specular_color = directional_light_.specular_color();
    imgui_input("Specular color", specular_color);
    directional_light_.set_specular_color(specular_color);
  }

  ImGui::Separator();

  {
    ImGui::Text("HDR");
    imgui_input("Exposure", exposure_);
  }
}

void BlinnPhongApplication::recreate_debug_quad_framebuffer(int new_width,
                                                            int new_height)
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

void BlinnPhongApplication::render_imgui_shadows()
{
  ImGui::Checkbox("Enable shadows", &is_shadows_enabled_);
  if (!is_shadows_enabled_)
  {
    return;
  }
  ImGui::Checkbox("Show cascades", &show_shadow_cascades_);

  imgui_input("Light size", light_size_);
  imgui_input("Bias", shadow_bias_min_);
  imgui_input("Smooth", smooth_shadows_);

  const int color_tex_width  = 1024;
  const int color_tex_height = 1024;
  recreate_debug_quad_framebuffer(color_tex_width, color_tex_height);

  // let the user select a cascade
  const auto selected_cascade = std::to_string(debug_selected_cascade_);
  if (ImGui::BeginCombo("Cascade", selected_cascade.c_str()))
  {
    for (std::size_t i = 0; i < cascade_frustums_.size(); ++i)
    {
      const auto is_selected = i == debug_selected_cascade_;
      const auto cascade_str = std::to_string(i);
      if (ImGui::Selectable(cascade_str.c_str(), is_selected))
      {
        debug_selected_cascade_ = i;
      }

      if (is_selected)
      {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  // render a debug quad with the selected cascade
  {
    debug_quad_framebuffer_->bind();
    glViewport(0, 0, color_tex_width, color_tex_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    depth_debug_shader_->bind();
    glActiveTexture(GL_TEXTURE0);
    shadow_tex_array_->bind();
    depth_debug_shader_->set_uniform("depth_tex", 0);
    depth_debug_shader_->set_uniform("layer",
                                     static_cast<int>(debug_selected_cascade_));
    glBindVertexArray(quad_vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    depth_debug_shader_->unbind();
    debug_quad_framebuffer_->unbind();
  }

  // display the cascade to the user
  const auto area = ImGui::GetContentRegionAvail();
  const auto tex = std::get<std::shared_ptr<GlTexture>>(
      debug_quad_framebuffer_->color_attachment(0));
  ImGui::Text("Cascades");
  ImGui::Image(reinterpret_cast<void *>(tex->id()), {area.x, area.x});
}

void BlinnPhongApplication::on_render_imgui()
{
  ImGui::ShowDemoWindow();

  if (ImGui::BeginTabBar("Renderer tabs"))
  {
    if (ImGui::BeginTabItem("General"))
    {
      render_imgui_general();
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Shadows"))
    {
      render_imgui_shadows();
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
}

void BlinnPhongApplication::on_key_callback(GLFWwindow * /*window*/,
                                            int key,
                                            int /*scancode*/,
                                            int action,
                                            int /*mods*/)
{
  if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
  {
    set_move_camera(true);
  }
  else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
  {
    set_move_camera(false);
  }
}

void BlinnPhongApplication::set_move_camera(bool value)
{
  if (value)
  {
    glfwSetInputMode(glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else
  {
    glfwSetInputMode(glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  is_move_camera_ = value;
}

void BlinnPhongApplication::move_camera(float delta_time)
{
  if (!is_move_camera_)
  {
    return;
  }

  if (key(GLFW_KEY_W) == GLFW_PRESS)
  {
    camera_.process_movement(CameraMovement::Forward, delta_time);
  }
  if (key(GLFW_KEY_S) == GLFW_PRESS)
  {
    camera_.process_movement(CameraMovement::Backward, delta_time);
  }
  if (key(GLFW_KEY_A) == GLFW_PRESS)
  {
    camera_.process_movement(CameraMovement::Left, delta_time);
  }
  if (key(GLFW_KEY_D) == GLFW_PRESS)
  {
    camera_.process_movement(CameraMovement::Right, delta_time);
  }
}

void BlinnPhongApplication::recreate_scene_framebuffer()
{
  FramebufferAttachmentCreateConfig color_config{};
  color_config.type_            = AttachmentType::Texture;
  color_config.format_          = GL_RGBA;
  color_config.internal_format_ = GL_RGBA16F;
  color_config.height_          = window_height_;
  color_config.width_           = window_width_;

  FramebufferAttachmentCreateConfig depth_config{};
  depth_config.type_            = AttachmentType::Renderbuffer;
  depth_config.format_          = GL_DEPTH_COMPONENT;
  depth_config.internal_format_ = GL_DEPTH_COMPONENT32F;
  depth_config.width_           = window_width_;
  depth_config.height_          = window_height_;

  FramebufferConfig config{};
  config.color_attachments_.push_back(color_config);
  config.depth_attachment_ = depth_config;

  scene_framebuffer_ = std::make_shared<GlFramebuffer>();
  scene_framebuffer_->attach(config);
}

void BlinnPhongApplication::calc_shadow_cascades_splits()
{
  assert(shadow_cascades_count_ > 0);
  cascade_frustums_.resize(shadow_cascades_count_);

  const auto near   = camera_near_;
  const auto lambda = 0.75f;
  const auto ratio  = camera_far_ / near;

  cascade_frustums_[0].near = near;

  for (std::size_t i = 0; i < cascade_frustums_.size(); ++i)
  {
    const auto si = i / static_cast<float>(cascade_frustums_.size());

    cascade_frustums_[i].near =
        lambda * (near * glm::pow(ratio, si)) +
        (1 - lambda) * (near + (camera_far_ - near) * si);

    cascade_frustums_[static_cast<int>(i) - 1].far =
        cascade_frustums_[i].near * 1.005f;
  }
  cascade_frustums_[cascade_frustums_.size() - 1].far = camera_far_;
}
