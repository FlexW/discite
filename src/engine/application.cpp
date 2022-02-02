#include "application.hpp"
#include "gl.hpp"
#include "log.hpp"
#include "time.hpp"

#include <cassert>
#include <cstdlib>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
} // namespace

Application::~Application() = default;

int Application::run()
{
  try
  {
    init();
    main_loop();
  }
  catch (const std::runtime_error &error)
  {
    LOG_ERROR() << "Unhandled exception: " << error.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void Application::init()
{
  Log::set_reporting_level(log_level_);

  if (!glfwInit())
  {
    throw std::runtime_error("Can not init GLFW");
  }

  glfwSetErrorCallback(glfw_error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_version_major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_version_minor);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  const auto opengl_debug{true};
  if (opengl_debug)
  {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  }

  const auto window_width{1280};
  const auto window_height{1024};
  const auto window_title{"Engine"};

  window_ = glfwCreateWindow(window_width,
                             window_height,
                             window_title,
                             nullptr,
                             nullptr);
  if (!window_)
  {
    throw std::runtime_error("Could not create GLFW window");
  }

  glfwSetWindowUserPointer(window_, this);
  glfwSetMonitorUserPointer(glfwGetPrimaryMonitor(), nullptr);

  glfwMakeContextCurrent(window_);

  // Enable vsync
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

  if (opengl_debug)
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
                         "Start debugging");
  }

  gl_dump_info();

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  init_imgui();
}

void Application::main_loop()
{
  auto last_time = current_time_millis();

  while (glfwWindowShouldClose(window_) == GLFW_FALSE && !is_close_)
  {
    glfwPollEvents();

    // Calculate delta time
    const auto delta_time = (current_time_millis() - last_time) / 1000.0f;
    last_time             = current_time_millis();

    on_update(delta_time);

    // render imgui
    {
      // start imgui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      on_render_imgui();
      // finish imgui frame
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window_);
  }

  shutdown_imgui();
}

void Application::on_window_framebuffer_size_callback(GLFWwindow * /*window*/,
                                                      int /*width*/,
                                                      int /*height*/)
{
}

void Application::on_window_close_callback(GLFWwindow * /*window*/) {}

void Application::on_key_callback(GLFWwindow * /*window*/,
                                  int /*key*/,
                                  int /*scancode*/,
                                  int /*action*/,
                                  int /*mods*/)
{
}

void Application::on_mouse_button_callback(GLFWwindow * /*window*/,
                                           int /*button*/,
                                           int /*action*/,
                                           int /*mods*/)
{
}

void Application::on_mouse_movement_callback(GLFWwindow * /*window*/,
                                             double /*x*/,
                                             double /*y*/)
{
}

void Application::window_framebuffer_size_callback(GLFWwindow *window,
                                                   int         width,
                                                   int         height)
{
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  assert(app);
  app->on_window_framebuffer_size_callback(window, width, height);
}

void Application::window_close_callback(GLFWwindow *window)
{
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  assert(app);
  app->on_window_close_callback(window);
}

void Application::key_callback(GLFWwindow *window,
                               int         key,
                               int         scancode,
                               int         action,
                               int         mods)
{
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  assert(app);
  app->on_key_callback(window, key, scancode, action, mods);
}

void Application::mouse_button_callback(GLFWwindow *window,
                                        int         button,
                                        int         action,
                                        int         mods)
{
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  assert(app);
  app->on_mouse_button_callback(window, button, action, mods);
}

void Application::mouse_movement_callback(GLFWwindow *window,
                                          double      x,
                                          double      y)
{
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  assert(app);
  app->on_mouse_movement_callback(window, x, y);
}

GLFWwindow *Application::glfw_window() const { return window_; }

int Application::key(int key) const { return glfwGetKey(window_, key); }

void Application::close() { is_close_ = true; }

void Application::init_imgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init("#version 460 core");
}

void Application::shutdown_imgui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Application::on_render_imgui() {}
