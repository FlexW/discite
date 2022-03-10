#pragma once

#include "gl.hpp"
#include "gl_shader.hpp"
#include "gl_vertex_array.hpp"
#include "gl_vertex_buffer.hpp"
#include "math.hpp"

#include <memory>
#include <vector>

namespace dc
{

class DebugDraw
{
public:
  DebugDraw();

  void
  draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color);

  void draw_line(const std::vector<glm::vec3> &line, const glm::vec3 &color);

  void submit(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix);

private:
  static constexpr std::size_t default_buffer_size{4000};

  std::vector<glm::vec3> lines_;
  std::vector<glm::vec3> colors_;

  std::unique_ptr<GlShader> gl_shader_;

  GlVertexArray                       vertex_array_;
  std::shared_ptr<GlVertexBuffer>     lines_vertex_buffer_;
  std::shared_ptr<GlVertexBuffer>     colors_vertex_buffer_;
};

} // namespace dc
