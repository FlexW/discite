#include "debug_draw.hpp"
#include "gl.hpp"
#include "gl_shader.hpp"
#include "gl_vertex_buffer.hpp"

#include <memory>

namespace dc
{

DebugDraw::DebugDraw()
{
  GlVertexBufferLayout layout_vec3;
  layout_vec3.push_float(3);

  lines_vertex_buffer_ =
      std::make_shared<GlVertexBuffer>(default_buffer_size, layout_vec3);
  colors_vertex_buffer_ =
      std::make_shared<GlVertexBuffer>(default_buffer_size, layout_vec3);

  vertex_array_.add_vertex_buffer(lines_vertex_buffer_);
  vertex_array_.add_vertex_buffer(colors_vertex_buffer_);

  gl_shader_ = std::make_unique<GlShader>();
  gl_shader_->init("shaders/line.vert", "shaders/line.frag");
}

void DebugDraw::draw_line(const glm::vec3 &from,
                          const glm::vec3 &to,
                          const glm::vec3 &color)
{
  lines_.push_back(from);
  lines_.push_back(to);
  colors_.push_back(color);
  colors_.push_back(color);
}

void DebugDraw::draw_line(const std::vector<glm::vec3> &line,
                          const glm::vec3              &color)
{
  for (std::size_t i = 1; i < line.size(); ++i)
  {
    lines_.push_back(line[i - 1]);
    colors_.push_back(color);

    lines_.push_back(line[i]);
    colors_.push_back(color);
  }
}

void DebugDraw::submit(const glm::mat4 & /*view_matrix*/,
                       const glm::mat4 & /*projection_matrix*/)
{
  if (lines_.size() == 0)
  {
    return;
  }

  // TODO: Fill vertex buffers and draw

  // GlVertexBufferLayout layout_vec3;
  // layout_vec3.push_float(3);

  // lines_vertex_buffer_->set_data(lines_, layout_vec3, GL_DYNAMIC_DRAW);
  // colors_vertex_buffer_->set_data(colors_, layout_vec3, GL_DYNAMIC_DRAW);

  // gl_shader_->bind();
  // gl_shader_->set_uniform("view_matrix", view_matrix);
  // gl_shader_->set_uniform("projection_matrix", projection_matrix);
  // draw(vertex_array_, GL_LINES);
  // gl_shader_->unbind();

  // lines_.clear();
  // colors_.clear();
}

} // namespace dc
