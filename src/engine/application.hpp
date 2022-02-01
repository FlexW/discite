#pragma once

#include "gl.hpp"
#include "log.hpp"

#include <GLFW/glfw3.h>

class Application
{
public:
  virtual ~Application();

  int run();

protected:
  virtual void init();
  virtual void on_update(float delta_time) = 0;
  virtual void on_render_imgui();

  virtual void on_window_framebuffer_size_callback(GLFWwindow *window,
                                                   int         width,
                                                   int         height);
  virtual void on_window_close_callback(GLFWwindow *window);

  virtual void on_key_callback(GLFWwindow *window,
                               int         key,
                               int         scancode,
                               int         action,
                               int         mods);

  virtual void on_mouse_button_callback(GLFWwindow *window,
                                        int         button,
                                        int         action,
                                        int         mods);

  virtual void
  on_mouse_movement_callback(GLFWwindow *window, double x, double y);

  void close();

  GLFWwindow *glfw_window() const;
  int         key(int key) const;

private:
  GLFWwindow *window_{};
  LogLevel    log_level_{LogLevel::Debug};

  bool is_close_{false};

  void main_loop();
  void init_imgui();
  void shutdown_imgui();

  static void
  window_framebuffer_size_callback(GLFWwindow *window, int width, int height);
  static void window_close_callback(GLFWwindow *window);

  static void
  key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

  static void
  mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

  static void mouse_movement_callback(GLFWwindow *window, double x, double y);
};
