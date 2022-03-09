#include "window.hpp"
#include "assert.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "gl.hpp"
#include "log.hpp"

#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>

namespace
{
using namespace dc;

constexpr auto opengl_version_major = 4;
constexpr auto opengl_version_minor = 6;

void glfw_error_callback(int error_code, const char *description)
{
  DC_LOG_ERROR() << "GLFW Error code: " << error_code
              << " Description: " << description;
}

void APIENTRY gl_debug_callback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei /*length*/,
                                const GLchar *msg,
                                const void * /*param*/)
{
  std::string source_str;
  switch (source)
  {
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    source_str = "WindowSys";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    source_str = "App";
    break;
  case GL_DEBUG_SOURCE_API:
    source_str = "OpenGL";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    source_str = "ShaderCompiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    source_str = "3rdParty";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    source_str = "Other";
    break;
  default:
    source_str = "Unknown";
  }

  std::string type_str;
  switch (type)
  {
  case GL_DEBUG_TYPE_ERROR:
    type_str = "Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    type_str = "Deprecated";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    type_str = "Undefined";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    type_str = "Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    type_str = "Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    type_str = "Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    type_str = "PushGrp";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    type_str = "PopGrp";
    break;
  case GL_DEBUG_TYPE_OTHER:
    type_str = "Other";
    break;
  default:
    type_str = "Unknown";
  }

  switch (severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:
    DC_LOG_ERROR() << source_str << " Type: " << type_str << " Id: " << id
                << " Message: " << msg;
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    DC_LOG_WARN() << source_str << " Type: " << type_str << " Id: " << id
               << " Message: " << msg;
    break;
  case GL_DEBUG_SEVERITY_LOW:
    DC_LOG_WARN() << source_str << " Type: " << type_str << " Id: " << id
               << " Message: " << msg;
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    DC_LOG_DEBUG() << source_str << " Type: " << type_str << " Id: " << id
                << " Message: " << msg;
    break;
  default:
    DC_LOG_WARN() << source_str << " Type: " << type_str << " Id: " << id
               << " Message: " << msg;
  }
}

void gl_dump_info()
{
  const auto renderer     = glGetString(GL_RENDERER);
  const auto vendor       = glGetString(GL_VENDOR);
  const auto version      = glGetString(GL_VERSION);
  const auto glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

  GLint major, minor, samples, sampleBuffers;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  glGetIntegerv(GL_SAMPLES, &samples);
  glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);

  GLint extensions_count = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &extensions_count);

  DC_LOG_INFO() << "GL Vendor: " << vendor;
  DC_LOG_INFO() << "GL Renderer: " << renderer;
  DC_LOG_INFO() << "GL Version: " << version;
  DC_LOG_INFO() << "GLSL Version: " << glsl_version;
  DC_LOG_INFO() << "MSAA samples: " << samples;
  DC_LOG_INFO() << "MSAA buffers: " << sampleBuffers;

  std::string extensions;
  for (GLint i = 0; i < extensions_count; ++i)
  {
    const auto extension = glGetStringi(GL_EXTENSIONS, i);
    if (i == 0)
    {
      extensions += reinterpret_cast<const char *>(extension);
    }
    else
    {
      extensions +=
          std::string(", ") + reinterpret_cast<const char *>(extension);
    }
  }
  DC_LOG_DEBUG() << "GL Extensions: " << extensions;
}

Key to_key(int key)
{
  switch (key)
  {
  case GLFW_KEY_A:
    return Key::A;

  case GLFW_KEY_B:
    return Key::B;

  case GLFW_KEY_C:
    return Key::C;

  case GLFW_KEY_D:
    return Key::D;

  case GLFW_KEY_E:
    return Key::E;

  case GLFW_KEY_F:
    return Key::F;

  case GLFW_KEY_G:
    return Key::G;

  case GLFW_KEY_H:
    return Key::H;

  case GLFW_KEY_I:
    return Key::I;

  case GLFW_KEY_J:
    return Key::J;

  case GLFW_KEY_K:
    return Key::K;

  case GLFW_KEY_L:
    return Key::L;

  case GLFW_KEY_M:
    return Key::M;

  case GLFW_KEY_N:
    return Key::N;

  case GLFW_KEY_O:
    return Key::O;

  case GLFW_KEY_Q:
    return Key::Q;

  case GLFW_KEY_R:
    return Key::R;

  case GLFW_KEY_S:
    return Key::S;

  case GLFW_KEY_T:
    return Key::T;

  case GLFW_KEY_V:
    return Key::V;

  case GLFW_KEY_W:
    return Key::W;

  case GLFW_KEY_X:
    return Key::X;

  case GLFW_KEY_Y:
    return Key::Y;

  case GLFW_KEY_Z:
    return Key::Z;

  case GLFW_KEY_LEFT_CONTROL:
    return Key::LeftControl;

  case GLFW_KEY_RIGHT_CONTROL:
    return Key::RightControl;

  case GLFW_KEY_LEFT_ALT:
    return Key::LeftAlt;

  case GLFW_KEY_RIGHT_ALT:
    return Key::RightAlt;

  case GLFW_KEY_LEFT_SHIFT:
    return Key::LeftShift;

  case GLFW_KEY_RIGHT_SHIFT:
    return Key::RightShift;

  case GLFW_KEY_LEFT_SUPER:
    return Key::LeftShift;

  case GLFW_KEY_RIGHT_SUPER:
    return Key::RightShift;

  case GLFW_KEY_TAB:
    return Key::Tab;

  case GLFW_KEY_LEFT:
    return Key::Left;

  case GLFW_KEY_RIGHT:
    return Key::Right;

  case GLFW_KEY_UP:
    return Key::Up;

  case GLFW_KEY_DOWN:
    return Key::Down;

  case GLFW_KEY_PAGE_DOWN:
    return Key::PageDown;

  case GLFW_KEY_PAGE_UP:
    return Key::PageUp;

  case GLFW_KEY_HOME:
    return Key::Home;

  case GLFW_KEY_END:
    return Key::End;

  case GLFW_KEY_INSERT:
    return Key::Insert;

  case GLFW_KEY_KP_ENTER:
    return Key::KeyPadEnter;

  case GLFW_KEY_SPACE:
    return Key::Space;

  case GLFW_KEY_BACKSPACE:
    return Key::Backspace;

  case GLFW_KEY_ENTER:
    return Key::Enter;

  case GLFW_KEY_ESCAPE:
    return Key::Escape;
  }

  DC_LOG_WARN() << "Unknown GLFW key: " << key;

  return Key::Undefined;
}

KeyAction to_key_action(int key_action)
{
  switch (key_action)
  {
  case GLFW_PRESS:
    return KeyAction::Press;
  case GLFW_RELEASE:
    return KeyAction::Release;
  case GLFW_REPEAT:
    return KeyAction::Repeat;
  }

  DC_LOG_WARN() << "Unknown GLFW action: " << key_action;

  return KeyAction::Undefined;
}

MouseButton to_mouse_button(int mouse_button)
{
  switch (mouse_button)
  {
  case GLFW_MOUSE_BUTTON_LEFT:
    return MouseButton::Left;
  case GLFW_MOUSE_BUTTON_RIGHT:
    return MouseButton::Right;
  }

  DC_LOG_WARN() << "Unknown GLFW mouse button: " << mouse_button;
  DC_FAIL("Unknown GLFW mouse button: {}", mouse_button);

  return MouseButton::Undefined;
}

MouseButtonAction to_mouse_button_action(int mouse_button_action)
{
  switch (mouse_button_action)
  {
  case GLFW_PRESS:
    return MouseButtonAction::Press;
  case GLFW_RELEASE:
    return MouseButtonAction::Release;
  case GLFW_REPEAT:
    return MouseButtonAction::Repeat;
  }

  DC_LOG_WARN() << "Unknown GLFW mouse button action: " << mouse_button_action;

  return MouseButtonAction::Undefined;
}

} // namespace

namespace dc
{

int to_glfw(Key key)
{
  switch (key)
  {
  case Key::A:
    return GLFW_KEY_A;

  case Key::B:
    return GLFW_KEY_B;

  case Key::C:
    return GLFW_KEY_C;

  case Key::D:
    return GLFW_KEY_D;

  case Key::E:
    return GLFW_KEY_E;

  case Key::F:
    return GLFW_KEY_F;

  case Key::G:
    return GLFW_KEY_G;

  case Key::H:
    return GLFW_KEY_H;

  case Key::I:
    return GLFW_KEY_I;

  case Key::J:
    return GLFW_KEY_J;

  case Key::K:
    return GLFW_KEY_K;

  case Key::L:
    return GLFW_KEY_L;

  case Key::M:
    return GLFW_KEY_M;

  case Key::N:
    return GLFW_KEY_N;

  case Key::O:
    return GLFW_KEY_O;

  case Key::P:
    return GLFW_KEY_P;

  case Key::Q:
    return GLFW_KEY_Q;

  case Key::R:
    return GLFW_KEY_R;

  case Key::S:
    return GLFW_KEY_S;

  case Key::T:
    return GLFW_KEY_T;

  case Key::U:
    return GLFW_KEY_U;

  case Key::V:
    return GLFW_KEY_V;

  case Key::W:
    return GLFW_KEY_W;

  case Key::X:
    return GLFW_KEY_X;

  case Key::Y:
    return GLFW_KEY_Y;

  case Key::Z:
    return GLFW_KEY_Z;

  case Key::Tab:
    return GLFW_KEY_TAB;

  case Key::Home:
    return GLFW_KEY_HOME;

  case Key::End:
    return GLFW_KEY_END;

  case Key::PageUp:
    return GLFW_KEY_PAGE_UP;

  case Key::PageDown:
    return GLFW_KEY_PAGE_DOWN;

  case Key::KeyPadEnter:
    return GLFW_KEY_KP_ENTER;

  case Key::LeftShift:
    return GLFW_KEY_LEFT_SHIFT;

  case Key::RightShift:
    return GLFW_KEY_RIGHT_SHIFT;

  case Key::RightSuper:
    return GLFW_KEY_RIGHT_SUPER;

  case Key::LeftSuper:
    return GLFW_KEY_LEFT_SUPER;

  case Key::LeftControl:
    return GLFW_KEY_LEFT_CONTROL;

  case Key::RightControl:
    return GLFW_KEY_LEFT_CONTROL;

  case Key::LeftAlt:
    return GLFW_KEY_LEFT_ALT;

  case Key::RightAlt:
    return GLFW_KEY_RIGHT_ALT;

  case Key::Left:
    return GLFW_KEY_LEFT;

  case Key::Right:
    return GLFW_KEY_RIGHT;

  case Key::Up:
    return GLFW_KEY_UP;

  case Key::Down:
    return GLFW_KEY_DOWN;

  case Key::Space:
    return GLFW_KEY_SPACE;

  case Key::Backspace:
    return GLFW_KEY_BACKSPACE;

  case Key::Delete:
    return GLFW_KEY_DELETE;

  case Key::Insert:
    return GLFW_KEY_INSERT;

  case Key::Enter:
    return GLFW_KEY_ENTER;

  case Key::Escape:
    return GLFW_KEY_ESCAPE;

  case Key::Undefined:
    return GLFW_KEY_UNKNOWN;
  }

  return GLFW_KEY_UNKNOWN;
}

int to_glfw(KeyAction action)
{
  switch (action)
  {
  case KeyAction::Press:
    return GLFW_PRESS;

  case KeyAction::Release:
    return GLFW_RELEASE;

  case KeyAction::Undefined:
    return GLFW_RELEASE;

  case KeyAction::Repeat:
    return GLFW_REPEAT;
  };
  DC_FAIL("No such key action");
}

int to_glfw(MouseButton button)
{
  switch (button)
  {
  case MouseButton::Left:
    return GLFW_MOUSE_BUTTON_LEFT;

  case MouseButton::Right:
    return GLFW_MOUSE_BUTTON_RIGHT;

  case MouseButton::Undefined:
    return GLFW_MOUSE_BUTTON_LEFT;
  }

  DC_FAIL("No such mouse button");
}

int to_glfw(MouseButtonAction action)
{
  switch (action)
  {
  case MouseButtonAction::Press:
    return GLFW_PRESS;

  case MouseButtonAction::Release:
    return GLFW_RELEASE;

  case MouseButtonAction::Undefined:
    return GLFW_RELEASE;

  case MouseButtonAction::Repeat:
    return GLFW_REPEAT;
  };
  DC_FAIL("No such mouse button action");
}

EventId WindowResizeEvent::id = 0xc5f4032c;

WindowResizeEvent::WindowResizeEvent(int width, int height)
    : Event{id},
      width_{width},
      height_{height}
{
}

EventId WindowCloseEvent::id = 0x5a1ac060;

WindowCloseEvent::WindowCloseEvent() : Event{id} {}

EventId KeyEvent::id = 0x7d186e9f;

KeyEvent::KeyEvent(Key key, KeyAction key_action, int scancode)
    : Event{id},
      key_{key},
      key_action_{key_action},
      scancode_{scancode}
{
}

EventId MouseButtonEvent::id = 0xf2f3ba14;

MouseButtonEvent::MouseButtonEvent(MouseButton       mouse_button,
                                   MouseButtonAction mouse_button_action)
    : Event{id},
      mouse_button_{mouse_button},
      mouse_button_action_{mouse_button_action}
{
}

EventId WindowFocusEvent::id = 0x753c7d2a;

WindowFocusEvent::WindowFocusEvent(bool focused) : Event{id}, focused_{focused}
{
}

EventId CursorEnterEvent::id = 0xd0223452;

CursorEnterEvent::CursorEnterEvent(bool entered) : Event{id}, entered_{entered}
{
}

EventId ScrollEvent::id = 0xe00815fa;

ScrollEvent::ScrollEvent(double x_offset, double y_offset)
    : Event{id},
      x_offset_{x_offset},
      y_offset_{y_offset}
{
}

EventId CharEvent::id = 0x0a7091f6;

CharEvent::CharEvent(unsigned int character) : Event{id}, character_{character}
{
}

EventId MonitorEvent::id = 0xdaabb915;

MonitorEvent::MonitorEvent(std::shared_ptr<Monitor> monitor, int event)
    : Event{id},
      monitor_{monitor},
      event_{event}
{
}

EventId MouseMovementEvent::id = 0x1b83b9ef;

MouseMovementEvent::MouseMovementEvent(double x,
                                       double y,
                                       double offset_x,
                                       double offset_y)
    : Event{id},
      x_{x},
      y_{y},
      offset_x_{offset_x},
      offset_y_{offset_y}
{
}

Monitor::Monitor(GLFWmonitor *handle) : handle_{handle} {}

GLFWmonitor *Monitor::handle() { return handle_; }

Window::Window(bool show_window)
{
  if (!glfwInit())
  {
    throw std::runtime_error("Can not init GLFW");
  }

  const auto engine = Engine::instance();
  const auto config = engine->config();

  glfwSetErrorCallback(glfw_error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_version_major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_version_minor);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE,
                 config->config_value_bool("Window", "resizable", true));

  const auto is_opengl_debug =
      config->config_value_bool("OpenGL", "debug", true);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, is_opengl_debug);

  window_width_  = config->config_value_int("Window", "width", window_width_);
  window_height_ = config->config_value_int("window", "height", window_height_);
  const auto window_title =
      config->config_value_string("Window", "title", "Engine");

  glfwWindowHint(GLFW_VISIBLE, show_window ? GLFW_TRUE : GLFW_FALSE);

  window_ = glfwCreateWindow(window_width_,
                             window_height_,
                             window_title.c_str(),
                             nullptr,
                             nullptr);
  if (!window_)
  {
    throw std::runtime_error("Could not create GLFW window");
  }

  glfwSetWindowUserPointer(window_, this);
  glfwSetMonitorUserPointer(glfwGetPrimaryMonitor(), nullptr);

  glfwMakeContextCurrent(window_);

  // enable vsync
  glfwSwapInterval(0);

  glfwSetWindowFocusCallback(window_, window_focus_callback);
  glfwSetCursorEnterCallback(window_, cursor_enter_callback);
  glfwSetCursorPosCallback(window_, mouse_movement_callback);
  glfwSetMouseButtonCallback(window_, mouse_button_callback);
  glfwSetScrollCallback(window_, scroll_callback);
  glfwSetKeyCallback(window_, key_callback);
  glfwSetCharCallback(window_, char_callback);
  glfwSetMonitorCallback(monitor_callback);
  glfwSetFramebufferSizeCallback(window_, window_framebuffer_size_callback);
  glfwSetWindowCloseCallback(window_, window_close_callback);

  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL())
  {
    DC_LOG_ERROR() << "GLAD could not load OpenGL";
    return;
  }

  if (is_opengl_debug)
  {
    glDebugMessageCallback(gl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE,
                          GL_DONT_CARE,
                          GL_DONT_CARE,
                          0,
                          nullptr,
                          GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_MARKER,
                         0,
                         GL_DEBUG_SEVERITY_NOTIFICATION,
                         -1,
                         "Debugging enabled");
  }

  gl_dump_info();

  // set some default OpenGL state
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool Window::is_close() { return glfwWindowShouldClose(window_); }

void Window::set_capture_mouse(bool value)
{
  if (value)
  {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else
  {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

bool Window::cursor_captured() const
{
  return glfwGetInputMode(window_, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

void Window::set_cursor_position(double x, double y)
{
  glfwSetCursorPos(window_, x, y);
}

std::pair<double, double> Window::cursor_position() const
{
  double mouse_x{};
  double mouse_y{};

  glfwGetCursorPos(window_, &mouse_x, &mouse_y);

  return {mouse_x, mouse_y};
}

std::pair<int, int> Window::position() const
{
  int window_x{};
  int window_y{};

  glfwGetWindowPos(window_, &window_x, &window_y);

  return {window_x, window_y};
}

int Window::width() const { return window_width_; }

int Window::height() const { return window_height_; }

int Window::framebuffer_width() const
{
  int w{};
  int h{};
  glfwGetFramebufferSize(window_, &w, &h);
  return w;
}

int Window::framebuffer_height() const
{
  int w{};
  int h{};
  glfwGetFramebufferSize(window_, &w, &h);
  return h;
}

void Window::dispatch_events() { glfwPollEvents(); }

void Window::swap_buffers() { glfwSwapBuffers(window_); }

double Window::time() const { return glfwGetTime(); }

void Window::window_framebuffer_size_callback(GLFWwindow *w,
                                              int         width,
                                              int         height)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_window_framebuffer_size(width, height);
}

void Window::window_close_callback(GLFWwindow *w)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_window_close();
}

void Window::key_callback(GLFWwindow *w,
                          int         key,
                          int         scancode,
                          int         action,
                          int         mods)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_key(key, scancode, action, mods);
}

void Window::mouse_button_callback(GLFWwindow *w,
                                   int         button,
                                   int         action,
                                   int         mods)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_mouse_button(button, action, mods);
}

void Window::mouse_movement_callback(GLFWwindow *w, double x, double y)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_mouse_movement(x, y);
}

void Window::monitor_callback(GLFWmonitor *m, int event)
{
  const auto monitor       = std::make_shared<Monitor>(m);
  const auto monitor_event = std::make_shared<MonitorEvent>(monitor, event);
  Engine::instance()->event_manager()->publish(monitor_event);
}

void Window::char_callback(GLFWwindow *w, unsigned int character)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_char(character);
}

void Window::scroll_callback(GLFWwindow *w, double xoffset, double yoffset)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_scroll(xoffset, yoffset);
}

void Window::cursor_enter_callback(GLFWwindow *w, int entered)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_cursor_enter(entered);
}

void Window::window_focus_callback(GLFWwindow *w, int focused)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  DC_ASSERT(window, "Window is nullptr");
  window->on_window_focus(focused);
}

void Window::on_char(char character)
{
  const auto event = std::make_shared<CharEvent>(character);
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_scroll(double xoffset, double yoffset)
{
  const auto event = std::make_shared<ScrollEvent>(xoffset, yoffset);
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_cursor_enter(int entered)
{
  const auto event = std::make_shared<CursorEnterEvent>(entered);
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_window_focus(int focused)
{
  const auto event = std::make_shared<WindowFocusEvent>(focused);
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_window_framebuffer_size(int width, int height)
{
  window_width_  = width;
  window_height_ = height;

  const auto event = std::make_shared<WindowResizeEvent>(width, height);
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_window_close()
{
  glfwSetWindowShouldClose(window_, GLFW_TRUE);

  const auto event = std::make_shared<WindowCloseEvent>();
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_key(int key, int scancode, int action, int mods)
{
  auto event =
      std::make_shared<KeyEvent>(to_key(key), to_key_action(action), scancode);
  if (mods & GLFW_MOD_CONTROL)
  {
    event->ctrl_pressed_ = true;
  }
  if (mods & GLFW_MOD_ALT)
  {
    event->alt_pressed_ = true;
  }
  if (mods & GLFW_MOD_SUPER)
  {
    event->super_pressed_ = true;
  }
  if (mods & GLFW_MOD_SHIFT)
  {
    event->shift_pressed_ = true;
  }
  if (mods & GLFW_MOD_CAPS_LOCK)
  {
    event->caps_lock_pressed_ = true;
  }
  if (mods & GLFW_MOD_NUM_LOCK)
  {
    event->num_lock_pressed_ = true;
  }
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_mouse_button(int button, int action, int /*mods*/)
{
  const auto event =
      std::make_shared<MouseButtonEvent>(to_mouse_button(button),
                                         to_mouse_button_action(action));
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_mouse_movement(double x, double y)
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

  const auto event =
      std::make_shared<MouseMovementEvent>(x, y, x_offset, y_offset);
  Engine::instance()->event_manager()->publish(event);
}

KeyAction Window::key(Key value) const
{
  const auto k      = to_glfw(value);
  const auto action = glfwGetKey(window_, k);
  return to_key_action(action);
}

GLFWwindow *Window::handle() const { return window_; }

bool Window::focused() const
{
  return glfwGetWindowAttrib(window_, GLFW_FOCUSED) != 0;
}

} // namespace dc
