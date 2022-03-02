#pragma once

#include "GLFW/glfw3.h"
#include "event.hpp"

#include <memory>
#include <utility>

struct GLFWwindow;
class Monitor;

enum class Key : int
{
  Undefined = 0,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  LeftControl,
  RightControl,
  LeftShift,
  RightShift,
  LeftAlt,
  RightAlt,
  LeftSuper,
  RightSuper,
  Tab,
  Left,
  Right,
  Up,
  Down,
  PageUp,
  PageDown,
  Home,
  End,
  Insert,
  Delete,
  Backspace,
  Space,
  Enter,
  Escape,
  KeyPadEnter,
};

enum class KeyAction
{
  Undefined,
  Press,
  Release,
  Repeat,
};

enum class MouseButton : int
{
  Undefined = 0,
  Left,
  Right,
};

enum class MouseButtonAction
{
  Undefined,
  Press,
  Release,
  Repeat,
};

enum class Cursor
{
  Arrow,
  IBeam,
  VResize,
  HResize,
  Hand,
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
  int       scancode_;

  bool ctrl_pressed_{false};
  bool alt_pressed_{false};
  bool shift_pressed_{false};
  bool super_pressed_{false};
  bool caps_lock_pressed_{false};
  bool num_lock_pressed_{false};

  KeyEvent(Key key, KeyAction key_action, int scancode);
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

class WindowFocusEvent : public Event
{
public:
  static EventId id;

  bool focused_;

  WindowFocusEvent(bool focused);
};

class CursorEnterEvent : public Event
{
public:
  static EventId id;

  bool entered_;

  CursorEnterEvent(bool entered);
};

class ScrollEvent : public Event
{
public:
  static EventId id;

  double x_offset_;
  double y_offset_;

  ScrollEvent(double x_offset, double y_offset);
};

class CharEvent : public Event
{
public:
  static EventId id;

  unsigned int character_;

  CharEvent(unsigned int character);
};

class MonitorEvent : public Event
{
public:
  static EventId id;

  std::shared_ptr<Monitor> monitor_;
  int event_;

  MonitorEvent(std::shared_ptr<Monitor> monitor, int event);
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

class Monitor
{
public:
  explicit Monitor(GLFWmonitor *handle);

  GLFWmonitor *handle();

private:
  GLFWmonitor *handle_{};
};

int to_glfw(Key key);
int to_glfw(KeyAction action);
int to_glfw(MouseButton button);
int to_glfw(MouseButtonAction action);

class Window
{
public:
  Window(bool show_window = true);

  bool is_close();

  void                      set_capture_mouse(bool value);
  bool                      cursor_captured() const;
  void                      set_cursor_position(double x, double y);
  std::pair<double, double> cursor_position() const;

  std::pair<int, int> position() const;

  int width() const;
  int height() const;

  int framebuffer_width() const;
  int framebuffer_height() const;

  KeyAction key(Key value) const;

  void dispatch_events();
  void swap_buffers();

  double time() const;

  bool focused() const;

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
  static void monitor_callback(GLFWmonitor *monitor, int event);
  static void char_callback(GLFWwindow *window, unsigned int character);
  static void
  scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
  static void cursor_enter_callback(GLFWwindow *window, int entered);
  static void window_focus_callback(GLFWwindow *window, int focused);

  void on_window_framebuffer_size(int width, int height);
  void on_window_close();
  void on_key(int key, int scancode, int action, int mods);
  void on_mouse_button(int button, int action, int mods);
  void on_mouse_movement(double x, double y);
  void on_char(char character);
  void on_scroll(double xoffset, double yoffset);
  void on_cursor_enter(int entered);
  void on_window_focus(int focused);
};
