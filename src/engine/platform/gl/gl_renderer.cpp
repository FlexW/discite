#include "gl_renderer.hpp"
#include "engine.hpp"
#include "gl.hpp"
#include "gl_helper.hpp"
#include "gl_shader.hpp"
#include "gl_vertex_buffer.hpp"
#include "graphic/renderer.hpp"
#include "graphic/vertex_buffer.hpp"
#include "platform/gl/gl_cube_texture.hpp"
#include "platform/gl/gl_framebuffer.hpp"
#include "platform/gl/gl_index_buffer.hpp"
#include "platform/gl/gl_renderbuffer.hpp"
#include "platform/gl/gl_shader_storage_buffer.hpp"
#include "platform/gl/gl_texture.hpp"
#include "platform/gl/gl_texture_view.hpp"
#include "platform/gl/gl_vertex_array.hpp"
#include <cstddef>
#include <memory>
#include <optional>

namespace
{

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
    DC_LOG_ERROR("{} Type: {} Id: {} Message: {}",
                 source_str,
                 type_str,
                 id,
                 msg);
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    DC_LOG_WARN("{} Type: {} Id: {} Message: {}",
                source_str,
                type_str,
                id,
                msg);
    break;
  case GL_DEBUG_SEVERITY_LOW:
    DC_LOG_WARN("{} Type: {} Id: {} Message: {}",
                source_str,
                type_str,
                id,
                msg);
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    DC_LOG_DEBUG("{} Type: {} Id: {} Message: {}",
                 source_str,
                 type_str,
                 id,
                 msg);
    break;
  default:
    DC_LOG_WARN("{} Type: {} Id: {} Message: {}",
                source_str,
                type_str,
                id,
                msg);
  }
}

void gl_dump_info()
{
  const auto renderer =
      reinterpret_cast<const char *>(glGetString(GL_RENDERER));
  const auto vendor  = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
  const auto version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  const auto glsl_version =
      reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

  GLint major, minor, samples, sampleBuffers;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  glGetIntegerv(GL_SAMPLES, &samples);
  glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);

  GLint extensions_count = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &extensions_count);

  DC_LOG_INFO("GL Vendor: {}", vendor);
  DC_LOG_INFO("GL Renderer: {}", renderer);
  DC_LOG_INFO("GL Version: {}", version);
  DC_LOG_INFO("GLSL Version: {}", glsl_version);
  DC_LOG_INFO("MSAA samples: {}", samples);
  DC_LOG_INFO("MSAA buffers: {}", sampleBuffers);

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
  DC_LOG_DEBUG("GL Extensions: {}", extensions);
}

} // namespace

namespace dc
{

GlRenderer::GlRenderer()
{
  if (!gladLoadGL())
  {
    DC_LOG_ERROR("GLAD could not load OpenGL");
    return;
  }

  const auto engine = Engine::instance();
  const auto config = engine->config();

  const auto is_opengl_debug =
      config->config_value_bool("OpenGL", "debug", true);

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
  glEnable(GL_MULTISAMPLE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  vertex_array_ = std::make_unique<GlVertexArray>();
}

std::shared_ptr<VertexBuffer>
GlRenderer::create_vertex_buffer(std::size_t               size,
                                 const VertexBufferLayout &layout,
                                 VertexBufferUsage         usage)
{
  return std::make_shared<GlVertexBuffer>(size, layout, usage);
}

std::shared_ptr<VertexBuffer>
GlRenderer::create_vertex_buffer(const void               *data,
                                 std::size_t               size,
                                 unsigned                  count,
                                 const VertexBufferLayout &layout,
                                 VertexBufferUsage         usage)
{
  return std::make_shared<GlVertexBuffer>(data, size, count, layout, usage);
}

std::shared_ptr<IndexBuffer>
GlRenderer::create_index_buffer(const std::vector<std::uint32_t> &indices)
{
  return std::make_shared<GlIndexBuffer>(indices);
}

std::shared_ptr<Shader> GlRenderer::create_shader(
    const std::filesystem::path    &vertex_shader_file_path,
    const std::filesystem::path    &fragment_shader_file_path,
    const std::vector<std::string> &preprocessor_defines)
{
  return std::make_shared<GlShader>(vertex_shader_file_path,
                                    fragment_shader_file_path,
                                    preprocessor_defines);
}

std::shared_ptr<Shader> GlRenderer::create_shader(
    const std::filesystem::path    &vertex_shader_file_path,
    const std::filesystem::path    &geometry_shader_file_path,
    const std::filesystem::path    &fragment_shader_file_path,
    const std::vector<std::string> &preprocessor_defines)
{
  return std::make_shared<GlShader>(vertex_shader_file_path,
                                    geometry_shader_file_path,
                                    fragment_shader_file_path,
                                    preprocessor_defines);
}

std::shared_ptr<Shader>
GlRenderer::create_shader(const std::filesystem::path &compute_shader_file_path,
                          const std::vector<std::string> &preprocessor_defines)
{
  return std::make_shared<GlShader>(compute_shader_file_path,
                                    preprocessor_defines);
}

std::shared_ptr<Texture> GlRenderer::create_texture(const TextureConfig &config)
{
  return std::make_shared<GlTexture>(config);
}

std::shared_ptr<CubeTexture>
GlRenderer::create_cube_texture(const CubeTextureConfig &config)
{
  return std::make_shared<GlCubeTexture>(config);
}

std::shared_ptr<TextureView>
GlRenderer::create_texture_view(const TextureViewConfig &config)
{
  return std::make_shared<GlTextureView>(config);
}

std::shared_ptr<Renderbuffer>
GlRenderer::create_renderbuffer(const RenderbufferConfig &config)
{
  return std::make_shared<GlRenderbuffer>(config);
}

std::shared_ptr<Framebuffer>
GlRenderer::create_framebuffer(const FramebufferConfig &config)
{
  return std::make_shared<GlFramebuffer>(config);
}

std::shared_ptr<ShaderStorageBuffer>
GlRenderer::create_shader_storage_buffer(void                   *data,
                                         std::size_t             size,
                                         ShaderStorageAccessMode access_mode)
{
  return std::make_shared<GlShaderStorageBuffer>(data, size, access_mode);
}

void GlRenderer::draw(const VertexBuffer &vertex_buffer,
                      const IndexBuffer  &index_buffer,
                      DrawMode            draw_mode)
{
  draw(std::vector<const VertexBuffer *>{&vertex_buffer},
       index_buffer,
       draw_mode);
}

void GlRenderer::draw(const std::vector<const VertexBuffer *> &vertex_buffers,
                      const IndexBuffer                       &index_buffer,
                      DrawMode                                 draw_mode)
{
  vertex_array_->bind_vertex_buffers(
      reinterpret_cast<const std::vector<const GlVertexBuffer *> &>(
          vertex_buffers));
  vertex_array_->bind_index_buffer(
      static_cast<const GlIndexBuffer &>(index_buffer));

  vertex_array_->bind();
  glDrawElements(to_gl(draw_mode),
                 index_buffer.count(),
                 GL_UNSIGNED_INT,
                 nullptr);
  vertex_array_->unbind();

  vertex_array_->unbind_vertex_buffers();
  vertex_array_->unbind_index_buffer();
}

void GlRenderer::draw(const VertexBuffer     &vertex_buffer,
                      std::optional<unsigned> count,
                      DrawMode                draw_mode)
{
  draw(std::vector<const VertexBuffer *>{&vertex_buffer}, count, draw_mode);
}

void GlRenderer::draw(const std::vector<const VertexBuffer *> &vertex_buffers,
                      std::optional<unsigned>                  count,
                      DrawMode                                 draw_mode)
{
  vertex_array_->bind_vertex_buffers(
      reinterpret_cast<const std::vector<const GlVertexBuffer *> &>(
          vertex_buffers));

  vertex_array_->bind();
  if (count.has_value())
  {
    glDrawArrays(to_gl(draw_mode), 0, count.value());
  }
  else
  {
    std::size_t all_vertices_count{0};
    for (std::size_t i{0}; i < vertex_buffers.size(); ++i)
    {
      all_vertices_count += vertex_buffers[i]->count();
    }
    glDrawArrays(to_gl(draw_mode), 0, all_vertices_count);
  }
  vertex_array_->unbind();

  vertex_array_->unbind_vertex_buffers();
  vertex_array_->unbind_index_buffer();
}

void GlRenderer::compute(std::uint32_t x_size,
                         std::uint32_t y_size,
                         std::uint32_t z_size)
{
  glDispatchCompute(x_size, y_size, z_size);
}

} // namespace dc
