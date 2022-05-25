#pragma once

#include "cube_texture.hpp"
#include "graphic/framebuffer.hpp"
#include "graphic/renderbuffer.hpp"
#include "graphic/shader_storage_buffer.hpp"
#include "graphic/texture_view.hpp"
#include "index_buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex_buffer.hpp"
#include "vertex_buffer_layout.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>

namespace dc
{

enum class DrawMode
{
  Triangles,
  Lines,
};

class Renderer
{
public:
  virtual ~Renderer() = default;

  template <typename T>
  std::shared_ptr<VertexBuffer>
  create_vertex_buffer(const std::vector<T>     &data,
                       const VertexBufferLayout &layout,
                       VertexBufferUsage         usage)
  {
    const auto size = data.size() * sizeof(T);
    return create_vertex_buffer(data.data(), size, data.size(), layout, usage);
  }

  virtual std::shared_ptr<VertexBuffer>
  create_vertex_buffer(std::size_t               size,
                       const VertexBufferLayout &layout,
                       VertexBufferUsage         usage) = 0;

  virtual std::shared_ptr<IndexBuffer>
  create_index_buffer(const std::vector<std::uint32_t> &indices) = 0;

  virtual std::shared_ptr<Shader>
  create_shader(const std::filesystem::path    &vertex_shader_file_path,
                const std::filesystem::path    &fragment_shader_file_path,
                const std::vector<std::string> &preprocessor_defines = {});

  virtual std::shared_ptr<Shader>
  create_shader(const std::filesystem::path    &vertex_shader_file_path,
                const std::filesystem::path    &geometry_shader_file_path,
                const std::filesystem::path    &fragment_shader_file_path,
                const std::vector<std::string> &preprocessor_defines = {});

  virtual std::shared_ptr<Shader>
  create_shader(const std::filesystem::path    &compute_shader_file_path,
                const std::vector<std::string> &preprocessor_defines = {});

  virtual std::shared_ptr<Texture>
  create_texture(const TextureConfig &config) = 0;

  std::shared_ptr<Texture>
  create_texture(const std::filesystem::path &file_path);

  virtual std::shared_ptr<CubeTexture>
  create_cube_texture(const CubeTextureConfig &config) = 0;

  virtual std::shared_ptr<TextureView>
  create_texture_view(const TextureViewConfig &config) = 0;

  virtual std::shared_ptr<Renderbuffer>
  create_renderbuffer(const RenderbufferConfig &config) = 0;

  virtual std::shared_ptr<Framebuffer>
  create_framebuffer(const FramebufferConfig &config) = 0;

  template <typename T>
  std::shared_ptr<ShaderStorageBuffer>
  create_shader_storage_buffer(const std::vector<T>   &data,
                               ShaderStorageAccessMode access_mode)
  {
    return create_shader_storage_buffer(data.data(),
                                        data.size() * sizeof(T),
                                        access_mode);
  }

  virtual void draw(const VertexBuffer &vertex_buffer,
                    const IndexBuffer  &index_buffer,
                    DrawMode            draw_mode = DrawMode::Triangles) = 0;

  virtual void draw(const std::vector<const VertexBuffer *> &vertex_buffers,
                    const IndexBuffer                       &index_buffer,
                    DrawMode draw_mode = DrawMode::Triangles) = 0;

  virtual void draw(const VertexBuffer     &vertex_buffer,
                    std::optional<unsigned> count = {},
                    DrawMode draw_mode            = DrawMode::Triangles) = 0;

  virtual void draw(const std::vector<const VertexBuffer *> &vertex_buffers,
                    std::optional<unsigned>                  count = {},
                    DrawMode draw_mode = DrawMode::Triangles) = 0;

  virtual void
  compute(std::uint32_t x_size, std::uint32_t y_size, std::uint32_t z_size) = 0;

protected:
  virtual std::shared_ptr<VertexBuffer>
  create_vertex_buffer(const void               *data,
                       std::size_t               size,
                       unsigned                  count,
                       const VertexBufferLayout &layout,
                       VertexBufferUsage         usage) = 0;

  virtual std::shared_ptr<ShaderStorageBuffer>
  create_shader_storage_buffer(void                   *data,
                               std::size_t             size,
                               ShaderStorageAccessMode access_mode) = 0;
};

} // namespace dc
