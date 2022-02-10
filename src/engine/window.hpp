#pragma once

#include "event.hpp"

#include <GLFW/glfw3.h>

enum class Key
{
  Undefined,
  Escape,
  W,
  S,
  A,
  D,
  LeftControl,
};

enum class KeyAction
{
  Undefined,
  Press,
  Release,
};

enum class MouseButton
{
  Undefined,
  Left,
  Right,
};

enum class MouseButtonAction
{
  Undefined,
  Press,
  Release,
};

class WindowResizeEvent : public Event
{
public:
  static EventId id;

  int width_;
  int height_;

  WindowResizeEvent(int width, int height);
};

class WindowCloseEvent : public Event
{
public:
  static EventId id;

  WindowCloseEvent();
};

class KeyEvent : public Event
{
public:
  static EventId id;

  Key       key_;
  KeyAction key_action_;

  KeyEvent(Key key, KeyAction key_action);
};

class MouseButtonEvent : public Event
{
public:
  static EventId id;

  MouseButton       mouse_button_;
  MouseButtonAction mouse_button_action_;

  MouseButtonEvent(MouseButton       mouse_button,
                   MouseButtonAction mouse_button_action);
};

class MouseMovementEvent : public Event
{
public:
  static EventId id;

  double x_;
  double y_;

  double offset_x_;
  double offset_y_;

  MouseMovementEvent(double x, double y, double offset_x, double offset_y);
};

class Window
{
public:
  Window();

  bool is_close();

  int width() const;
  int height() const;

  KeyAction key(Key value) const;

  void dispatch_events();
  void swap_buffers();

  GLFWwindow *handle() const;

private:
  GLFWwindow *window_{};

  int window_width_{1280};
  int window_height_{720};

  bool   is_mouse_first_move_{true};
  double mouse_last_x_{0.0};
  double mouse_last_y_{0.0};

  static void
  window_framebuffer_size_callback(GLFWwindow *window, int width, int height);
  static void window_close_callback(GLFWwindow *window);

  static void
  key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

  static void
  mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

  static void mouse_movement_callback(GLFWwindow *window, double x, double y);

  void on_window_framebuffer_size_callback(int width, int height);

  void on_window_close_callback();

  void on_key_callback(int key, int scancode, int action, int mods);

  void on_mouse_button_callback(int button, int action, int mods);

  void on_mouse_movement_callback(double x, double y);
};
