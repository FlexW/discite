#pragma once

#include "engine/application.hpp"
#include "engine/camera.hpp"
#include "engine/directional_light.hpp"
#include "engine/gl_shader.hpp"
#include "engine/mesh.hpp"
#include "engine/texture_cache.hpp"

#include <memory>

class BlinnPhongApplication : public Application
{
protected:
  void init() override;
  void on_update(float delta_time) override;

  void on_window_framebuffer_size_callback(GLFWwindow *window,
                                           int         width,
                                           int         height) override;

  void
  on_mouse_movement_callback(GLFWwindow *window, double x, double y) override;

  void on_window_close_callback(GLFWwindow *window) override;

private:
  DirectionalLight directional_light_;
  TextureCache     texture_cache_;

  bool   is_mouse_first_move_{true};
  double mouse_last_x_{0.0};
  double mouse_last_y_{0.0};

  Camera camera_;
  float  camera_near_{0.1f};
  float  camera_far_{8000.0f};

  glm::mat4 projection_matrix_{1.0f};

  std::shared_ptr<GlShader> model_shader_;
  std::shared_ptr<Model>    model_;

  int window_width_{0};
  int window_height_{0};

  void recalculate_projection_matrix();
};
