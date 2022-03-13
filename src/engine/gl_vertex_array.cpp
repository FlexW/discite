#include "gl_vertex_array.hpp"
#include "assert.hpp"

namespace dc
{

GlVertexArray::GlVertexArray() { glCreateVertexArrays(1, &id_); }

GlVertexArray::~GlVertexArray() { glDeleteVertexArrays(1, &id_); }

void GlVertexArray::add_vertex_buffer(
    std::shared_ptr<GlVertexBuffer> vertex_buffer)
{
  glVertexArrayVertexBuffer(id_,
                            static_cast<GLsizei>(vertex_buffers_.size()),
                            vertex_buffer->id(),
                            0,
                            vertex_buffer->layout().size());

  const auto layout          = vertex_buffer->layout();
  const auto layout_elements = layout.elements();

  GLuint offset{0};

  for (std::size_t i = 0; i < layout_elements.size(); ++i)
  {
    const auto layout_element = layout_elements[i];

    glEnableVertexArrayAttrib(id_, binding_point_);

    switch (layout_element.type)
    {
    case GL_FLOAT:
      glVertexArrayAttribFormat(id_,
                                binding_point_,
                                layout_element.count,
                                layout_element.type,
                                GL_FALSE,
                                offset);
      break;

    case GL_INT:
      glVertexArrayAttribIFormat(id_,
                                 binding_point_,
                                 layout_element.count,
                                 layout_element.type,
                                 offset);
      break;

    default:
      DC_FAIL("Can not handle Glsl type");
    }

    glVertexArrayAttribBinding(id_, binding_point_, static_cast<GLsizei>(vertex_buffers_.size()));

    offset += layout_element.size;
    ++binding_point_;
  }

  vertex_buffers_.push_back(vertex_buffer);
}

void GlVertexArray::set_index_buffer(
    std::shared_ptr<GlIndexBuffer> index_buffer)
{
  glVertexArrayElementBuffer(id_, index_buffer->id());
  index_buffer_ = index_buffer;
}

void GlVertexArray::bind() const { glBindVertexArray(id_); }

void GlVertexArray::unbind() const { glBindVertexArray(0); }

GlIndexBuffer *GlVertexArray::index_buffer() const
{
  return index_buffer_.get();
}

GLsizei GlVertexArray::vertex_count() const
{
  if (vertex_buffers_[0])
  {
    return vertex_buffers_[0]->count();
  }
  return 0;
}

} // namespace dc
