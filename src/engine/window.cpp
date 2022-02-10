#include "window.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "gl.hpp"
#include "log.hpp"

#include <cassert>
#include <stdexcept>

namespace
{
constexpr auto opengl_version_major = 4;
constexpr auto opengl_version_minor = 6;

void glfw_error_callback(int error_code, const char *description)
{
  LOG_ERROR() << "GLFW Error code: " << error_code
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
    LOG_ERROR() << source_str << " Type: " << type_str << " Id: " << id
                << " Message: " << msg;
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    LOG_WARN() << source_str << " Type: " << type_str << " Id: " << id
               << " Message: " << msg;
    break;
  case GL_DEBUG_SEVERITY_LOW:
    LOG_WARN() << source_str << " Type: " << type_str << " Id: " << id
               << " Message: " << msg;
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    LOG_DEBUG() << source_str << " Type: " << type_str << " Id: " << id
                << " Message: " << msg;
    break;
  default:
    LOG_WARN() << source_str << " Type: " << type_str << " Id: " << id
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

  LOG_INFO() << "GL Vendor: " << vendor;
  LOG_INFO() << "GL Renderer: " << renderer;
  LOG_INFO() << "GL Version: " << version;
  LOG_INFO() << "GLSL Version: " << glsl_version;
  LOG_INFO() << "MSAA samples: " << samples;
  LOG_INFO() << "MSAA buffers: " << sampleBuffers;

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
  LOG_DEBUG() << "GL Extensions: " << extensions;
}

Key to_key(int key)
{
  switch (key)
  {
  case GLFW_KEY_W:
    return Key::W;
  case GLFW_KEY_S:
    return Key::S;
  case GLFW_KEY_A:
    return Key::A;
  case GLFW_KEY_D:
    return Key::D;
  case GLFW_KEY_LEFT_CONTROL:
    return Key::LeftControl;
  case GLFW_KEY_ESCAPE:
    return Key::Escape;
  }

  LOG_WARN() << "Unknown key: " << key;

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
  }

  LOG_WARN() << "Unknown GLFW action: " << key_action;

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

  LOG_WARN() << "Unknown GLFW mouse button: " << mouse_button;

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
  }

  LOG_WARN() << "Unknown GLFW mouse button action: " << mouse_button_action;

  return MouseButtonAction::Undefined;
}

int to_glfw(Key key)
{
  switch (key)
  {
  case Key::W:
    return GLFW_KEY_W;

  case Key::S:
    return GLFW_KEY_S;

  case Key::A:
    return GLFW_KEY_A;

  case Key::D:
    return GLFW_KEY_D;

  case Key::LeftControl:
    return GLFW_KEY_LEFT_CONTROL;

  case Key::Escape:
    return GLFW_KEY_ESCAPE;

  case Key::Undefined:
    return GLFW_KEY_UNKNOWN;
  }

  return GLFW_KEY_UNKNOWN;
}

} // namespace

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

KeyEvent::KeyEvent(Key key, KeyAction key_action)
    : Event{id},
      key_{key},
      key_action_{key_action}
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

Window::Window()
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

  glfwSetFramebufferSizeCallback(window_, window_framebuffer_size_callback);
  glfwSetCursorPosCallback(window_, mouse_movement_callback);
  glfwSetMouseButtonCallback(window_, mouse_button_callback);
  glfwSetKeyCallback(window_, key_callback);
  glfwSetWindowCloseCallback(window_, window_close_callback);

  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL())
  {
    LOG_ERROR() << "GLAD could not load OpenGL";
    return;
  }

  glEnable(GL_SAMPLES);
  glfwWindowHint(GLFW_SAMPLES, 0);

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

int Window::width() const { return window_width_; }

int Window::height() const { return window_height_; }

void Window::dispatch_events() { glfwPollEvents(); }

void Window::swap_buffers() { glfwSwapBuffers(window_); }

void Window::window_framebuffer_size_callback(GLFWwindow *w,
                                              int         width,
                                              int         height)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  assert(window);
  window->on_window_framebuffer_size_callback(width, height);
}

void Window::window_close_callback(GLFWwindow *w)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  assert(window);
  window->on_window_close_callback();
}

void Window::key_callback(GLFWwindow *w,
                          int         key,
                          int         scancode,
                          int         action,
                          int         mods)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  assert(window);
  window->on_key_callback(key, scancode, action, mods);
}

void Window::mouse_button_callback(GLFWwindow *w,
                                   int         button,
                                   int         action,
                                   int         mods)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  assert(window);
  window->on_mouse_button_callback(button, action, mods);
}

void Window::mouse_movement_callback(GLFWwindow *w, double x, double y)
{
  auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
  assert(window);
  window->on_mouse_movement_callback(x, y);
}

void Window::on_window_framebuffer_size_callback(int width, int height)
{
  window_width_  = width;
  window_height_ = height;

  const auto event = std::make_shared<WindowResizeEvent>(width, height);
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_window_close_callback()
{
  glfwSetWindowShouldClose(window_, GLFW_TRUE);

  const auto event = std::make_shared<WindowCloseEvent>();
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_key_callback(int key,
                             int /*scancode*/,
                             int action,
                             int /*mods*/)
{
  const auto event =
      std::make_shared<KeyEvent>(to_key(key), to_key_action(action));
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_mouse_button_callback(int button, int action, int /*mods*/)
{
  const auto event =
      std::make_shared<MouseButtonEvent>(to_mouse_button(button),
                                         to_mouse_button_action(action));
  Engine::instance()->event_manager()->publish(event);
}

void Window::on_mouse_movement_callback(double x, double y)
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
