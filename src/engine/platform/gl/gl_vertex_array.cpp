#include "gl_vertex_array.hpp"
#include "assert.hpp"
#include "gl_helper.hpp"
#include "graphic/vertex_buffer_layout.hpp"

namespace dc
{

GlVertexArray::GlVertexArray() { glCreateVertexArrays(1, &id_); }

GlVertexArray::~GlVertexArray() { glDeleteVertexArrays(1, &id_); }

void GlVertexArray::bind_vertex_buffers(
    const std::vector<const GlVertexBuffer *> &vertex_buffers)
{
  unsigned binding_point{0};

  for (std::size_t i{0}; i < vertex_buffers.size(); ++i)
  {
    const auto &vertex_buffer = *vertex_buffers[i];

    glVertexArrayVertexBuffer(id_,
                              static_cast<GLsizei>(i),
                              vertex_buffer.id(),
                              0,
                              vertex_buffer.layout().size());

    const auto layout          = vertex_buffer.layout();
    const auto layout_elements = layout.elements();

    GLuint offset{0};

    for (std::size_t i = 0; i < layout_elements.size(); ++i)
    {
      const auto layout_element = layout_elements[i];

      glEnableVertexArrayAttrib(id_, binding_point);

      switch (layout_element.type)
      {
      case VertexBufferLayoutElementType::Float:
        glVertexArrayAttribFormat(id_,
                                  binding_point,
                                  layout_element.count,
                                  to_gl(layout_element.type),
                                  GL_FALSE,
                                  offset);
        break;

      case VertexBufferLayoutElementType::Int:
        glVertexArrayAttribIFormat(id_,
                                   binding_point,
                                   layout_element.count,
                                   to_gl(layout_element.type),
                                   offset);
        break;

      default:
        DC_FAIL("Can not handle Glsl type");
      }

      glVertexArrayAttribBinding(id_, binding_point, static_cast<GLsizei>(i));

      offset += layout_element.size;
      ++binding_point;
    }
  }
}

void GlVertexArray::unbind_vertex_buffers()
{
  for (int i{0}; i < 20; ++i)
  {
    glDisableVertexArrayAttrib(id_, i);
  }
}

// void GlVertexArray::set_index_buffer(
//     std::shared_ptr<GlIndexBuffer> index_buffer)
// {
//   glVertexArrayElementBuffer(id_, index_buffer->id());
//   index_buffer_ = index_buffer;
// }

void GlVertexArray::bind_index_buffer(const GlIndexBuffer &index_buffer)
{
  glVertexArrayElementBuffer(id_, index_buffer.id());
}

void GlVertexArray::unbind_index_buffer()
{
  glVertexArrayElementBuffer(id_, 0);
}

void GlVertexArray::bind() const { glBindVertexArray(id_); }

void GlVertexArray::unbind() const { glBindVertexArray(0); }

// GlIndexBuffer *GlVertexArray::index_buffer() const
// {
//   return index_buffer_.get();
// }

// GLsizei GlVertexArray::vertex_count() const
// {
//   if (vertex_buffers_[0])
//   {
//     return vertex_buffers_[0]->count();
//   }
//   return 0;
// }

} // namespace dc
