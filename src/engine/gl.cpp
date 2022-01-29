
#include "gl.hpp"
#include "gl_vertex_array.hpp"

void draw(const GlVertexArray &vertex_array, GLenum mode)
{
  vertex_array.bind();

  const auto index_buffer = vertex_array.index_buffer();
  if (index_buffer)
  {
    glDrawElements(mode, index_buffer->count(), GL_UNSIGNED_INT, nullptr);
  }
  else
  {
    glDrawArrays(mode, 0, vertex_array.vertex_count());
  }

  vertex_array.unbind();
}
