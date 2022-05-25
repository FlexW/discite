#pragma once

#include "gl_vertex_array.hpp"
#include "gl_vertex_buffer.hpp"
#include "graphic/index_buffer.hpp"
#include "graphic/renderer.hpp"
#include "graphic/vertex_buffer.hpp"

#include <memory>
#include <optional>

namespace dc
{

class GlRenderer : public Renderer
{
public:
  GlRenderer();

  std::shared_ptr<VertexBuffer>
  create_vertex_buffer(std::size_t               size,
                       const VertexBufferLayout &layout,
                       VertexBufferUsage         usage) override;

  std::shared_ptr<IndexBuffer>
  create_index_buffer(const std::vector<std::uint32_t> &indices) override;

  std::shared_ptr<Shader> create_shader(
      const std::filesystem::path    &vertex_shader_file_path,
      const std::filesystem::path    &fragment_shader_file_path,
      const std::vector<std::string> &preprocessor_defines = {}) override;

  std::shared_ptr<Shader> create_shader(
      const std::filesystem::path    &vertex_shader_file_path,
      const std::filesystem::path    &geometry_shader_file_path,
      const std::filesystem::path    &fragment_shader_file_path,
      const std::vector<std::string> &preprocessor_defines = {}) override;

  std::shared_ptr<Shader> create_shader(
      const std::filesystem::path    &compute_shader_file_path,
      const std::vector<std::string> &preprocessor_defines = {}) override;

  std::shared_ptr<Texture> create_texture(const TextureConfig &config) override;

  std::shared_ptr<CubeTexture>
  create_cube_texture(const CubeTextureConfig &config) override;

  std::shared_ptr<TextureView>
  create_texture_view(const TextureViewConfig &config) override;

  std::shared_ptr<Renderbuffer>
  create_renderbuffer(const RenderbufferConfig &config) override;

  std::shared_ptr<Framebuffer>
  create_framebuffer(const FramebufferConfig &config) override;

  void draw(const VertexBuffer &vertex_buffer,
            const IndexBuffer  &index_buffer,
            DrawMode            draw_mode = DrawMode::Triangles) override;

  void draw(const std::vector<const VertexBuffer *> &vertex_buffers,
            const IndexBuffer                       &index_buffer,
            DrawMode draw_mode = DrawMode::Triangles) override;

  void draw(const VertexBuffer     &vertex_buffer,
            std::optional<unsigned> count     = {},
            DrawMode                draw_mode = DrawMode::Triangles) override;

  void draw(const std::vector<const VertexBuffer *> &vertex_buffers,
            std::optional<unsigned>                  count = {},
            DrawMode draw_mode = DrawMode::Triangles) override;

  void compute(std::uint32_t x_size,
               std::uint32_t y_size,
               std::uint32_t z_size) override;

protected:
  std::shared_ptr<VertexBuffer>
  create_vertex_buffer(const void               *data,
                       std::size_t               size,
                       unsigned                  count,
                       const VertexBufferLayout &layout,
                       VertexBufferUsage         usage) override;

  std::shared_ptr<ShaderStorageBuffer>
  create_shader_storage_buffer(void                   *data,
                               std::size_t             size,
                               ShaderStorageAccessMode access_mode) override;

private:
  std::unique_ptr<GlVertexArray> vertex_array_{};
};

} // namespace dc
