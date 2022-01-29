#include "blinn_phong_application.hpp"
#include "engine/application.hpp"
#include "engine/gl_shader.hpp"
#include "engine/gl_texture.hpp"
#include "engine/math.hpp"
#include "engine/mesh.hpp"

#include <memory>

void BlinnPhongApplication::init()
{
  Application::init();

  // load sponza
  texture_cache_.set_import_path(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF");
  model_ = std::make_shared<Model>();
  model_->load_from_file(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
      texture_cache_);

  // reset path
  texture_cache_.set_import_path("data/textures");

  model_shader_ = std::make_shared<GlShader>();
  model_shader_->init("shaders/blinn_phong.vert", "shaders/blinn_phong.frag");

  recalculate_projection_matrix();
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

  camera_.process_rotation(x_offset, y_offset);
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

  // perform the rendering
  glViewport(0, 0, window_width_, window_height_);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glClearColor(0.3f, 0.81f, 0.92f, 1.0f);

  const auto view_matrix = camera_.view_matrix();

  model_shader_->bind();
  model_shader_->set_uniform("view_matrix", view_matrix);
  model_shader_->set_uniform("projection_matrix", projection_matrix_);

  model_shader_->set_uniform("point_light_count", 0);
  model_shader_->set_uniform("spot_light_count", 0);

  model_shader_->set_uniform("directional_light_enabled", 1);
  model_shader_->set_uniform(
      "directional_light.direction",
      glm::vec3{view_matrix * glm::vec4{directional_light_.direction(), 0.0f}});
  model_shader_->set_uniform("directional_light.ambient_color",
                             directional_light_.ambient_color());
  model_shader_->set_uniform("directional_light.diffuse_color",
                             directional_light_.diffuse_color());
  model_shader_->set_uniform("directional_light.specular_color",
                             directional_light_.specular_color());

  const auto model_matrix = glm::translate(glm::mat4{1.0f}, model_->position());
  model_shader_->set_uniform("model_matrix", model_matrix);

  const auto &meshes = model_->meshes();
  for (const auto &mesh : meshes)
  {
    int texture_slot = 0;

    const auto material = mesh->material();

    if (material->ambient_texture())
    {
      const auto ambient_texture = material->ambient_texture();
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      ambient_texture->bind();
      model_shader_->set_uniform("in_ambient_tex", texture_slot);
      model_shader_->set_uniform("ambient_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      model_shader_->set_uniform("in_ambient_color", material->ambient_color());
      model_shader_->set_uniform("ambient_tex_enabled", false);
    }

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
      model_shader_->set_uniform("in_diffuse_color", material->diffuse_color());
      model_shader_->set_uniform("diffuse_tex_enabled", false);
    }

    if (material->specular_texture())
    {
      const auto specular_texture = material->specular_texture();
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      specular_texture->bind();
      model_shader_->set_uniform("in_specular_tex", texture_slot);
      model_shader_->set_uniform("specular_tex_enabled", true);
      ++texture_slot;
    }
    else
    {
      model_shader_->set_uniform("in_specular_color",
                                 material->specular_color());
      model_shader_->set_uniform("specular_tex_enabled", false);
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

    model_shader_->set_uniform("specular_power", material->specular_power());

    const auto vertex_array = mesh->vertex_array();
    draw(*vertex_array, GL_TRIANGLES);
  }

  model_shader_->unbind();
}

void BlinnPhongApplication::recalculate_projection_matrix()
{
  projection_matrix_ =
      glm::perspective(glm::radians(camera_.zoom()),
                       static_cast<float>(window_width_) / window_height_,
                       camera_near_,
                       camera_far_);
}
