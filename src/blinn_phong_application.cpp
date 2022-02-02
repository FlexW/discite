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

  // enable the cursor
  set_move_camera(false);

  // get the window size
  glfwGetFramebufferSize(glfw_window(), &window_width_, &window_height_);

  // load cubes
  // texture_cache_.set_import_path("external/deps/src/cubes");
  // model_ = std::make_shared<Model>();
  // model_->load_from_file("external/deps/src/cubes/cubes.obj",
  // texture_cache_);

  // load sponza
  texture_cache_.set_import_path(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF");
  auto sponza_model = std::make_shared<Model>();
  sponza_model->load_from_file(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
      texture_cache_);
  add_model(sponza_model);

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

  // create framebuffer for shadow map
  shadow_tex_array_ = std::make_shared<GlTextureArray>();
  TextureArrayData texture_array_data{
      GL_DEPTH_COMPONENT32F,
      GL_DEPTH_COMPONENT,
      shadow_tex_width_,
      shadow_tex_height_,
      static_cast<int>(shadow_cascades_levels_.size()) + 1};
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
}

void BlinnPhongApplication::on_window_framebuffer_size_callback(
    GLFWwindow * /*window*/,
    int width,
    int height)
{
  window_width_  = width;
  window_height_ = height;

  recalculate_projection_matrix();
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
  for (std::size_t i = 0; i < shadow_cascades_levels_.size() + 1; ++i)
  {
    if (i == 0)
    {
      matrices.push_back(
          calc_light_space_matrix(camera_near_, shadow_cascades_levels_[i]));
    }
    else if (i < shadow_cascades_levels_.size())
    {
      matrices.push_back(calc_light_space_matrix(shadow_cascades_levels_[i - 1],
                                                 shadow_cascades_levels_[i]));
    }
    else
    {
      matrices.push_back(
          calc_light_space_matrix(shadow_cascades_levels_[i - 1], camera_far_));
    }
  }

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
  {
    shadow_framebuffer_->bind();

    glViewport(0, 0, shadow_tex_width_, shadow_tex_height_);
    glClear(GL_DEPTH_BUFFER_BIT);

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

  // // render debug quad
  // {
  //   glViewport(0, 0, window_width_, window_height_);
  //   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  //   glClearColor(0.3f, 0.81f, 0.92f, 1.0f);

  //   depth_debug_shader_->bind();
  //   glActiveTexture(GL_TEXTURE0);
  //   shadow_tex_array_->bind();
  //   depth_debug_shader_->set_uniform("depth_tex", 0);
  //   // depth_debug_shader_->set_uniform("near_plane");
  //   // depth_debug_shader_->set_uniform("far_plane");
  //   depth_debug_shader_->set_uniform("layer", 2);
  //   glBindVertexArray(quad_vertex_array_);
  //   glDrawArrays(GL_TRIANGLES, 0, 6);
  //   glBindVertexArray(0);
  //   depth_debug_shader_->unbind();
  // }
  // return;

  // render the model
  {
    int global_texture_slot = 0;

    glViewport(0, 0, window_width_, window_height_);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glClearColor(sky_color_.r, sky_color_.g, sky_color_.b, 1.0f);

    const auto view_matrix = camera_.view_matrix();

    model_shader_->bind();
    model_shader_->set_uniform("view_matrix", view_matrix);
    model_shader_->set_uniform("projection_matrix", projection_matrix_);

    model_shader_->set_uniform("point_light_count", 0);
    model_shader_->set_uniform("spot_light_count", 0);

    model_shader_->set_uniform("directional_light_enabled", 1);
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

    model_shader_->set_uniform("directional_light_shadow_enabled", true);
    model_shader_->set_uniform("far_plane", camera_far_);

    for (std::size_t i = 0; i < light_space_matrices.size(); ++i)
    {
      model_shader_->set_uniform("light_space_matrices[" + std::to_string(i) +
                                     "]",
                                 light_space_matrices[i]);
    }

    for (std::size_t i = 0; i < shadow_cascades_levels_.size(); ++i)
    {
      model_shader_->set_uniform("cascades_plane_distances[" +
                                     std::to_string(i) + "]",
                                 shadow_cascades_levels_[i]);
    }

    glActiveTexture(GL_TEXTURE0 + global_texture_slot);
    shadow_tex_array_->bind();
    model_shader_->set_uniform("directional_light_shadow_tex",
                               global_texture_slot);
    ++global_texture_slot;

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
          model_shader_->set_uniform("normal_tex_enabled", false);
        }

        model_shader_->set_uniform("specular_power",
                                   material->specular_power());

        const auto vertex_array = mesh->vertex_array();
        draw(*vertex_array, GL_TRIANGLES);
      }
    }

    model_shader_->unbind();
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

void BlinnPhongApplication::on_render_imgui()
{
  ImGui::Begin("Debug");
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

  ImGui::End();
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
