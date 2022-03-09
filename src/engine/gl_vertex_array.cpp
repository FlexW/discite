#include "gl_vertex_array.hpp"

#include <cassert>

namespace dc
{

GlVertexArray::GlVertexArray() { glCreateVertexArrays(1, &id_); }

GlVertexArray::~GlVertexArray() { glDeleteVertexArrays(1, &id_); }

void GlVertexArray::add_vertex_buffer(
    std::shared_ptr<GlVertexBuffer> vertex_buffer)
{
  bind();
  vertex_buffer->bind();

  const auto layout          = vertex_buffer->layout();
  const auto layout_elements = layout.elements();

  GLintptr offset{0};

  for (std::size_t i = 0; i < layout_elements.size(); ++i)
  {
    const auto layout_element = layout_elements[i];
    const auto binding_point  = vertex_buffers_.size() + i;

    glEnableVertexAttribArray(binding_point);

    switch (layout_element.type)
    {
    case GL_FLOAT:
      glVertexAttribPointer(binding_point,
                            layout_element.count,
                            layout_element.type,
                            false,
                            layout.size(),
                            reinterpret_cast<const void *>(offset));
      break;

    case GL_INT:
      glVertexAttribIPointer(binding_point,
                             layout_element.count,
                             GL_INT,
                             layout_element.size,
                             reinterpret_cast<const void *>(offset));
      break;

    default:
      assert(0 && "Can not handle Glsl type");
    }

    offset += layout_element.size;
  }

  vertex_buffers_.push_back(vertex_buffer);

  vertex_buffer->unbind();
  unbind();
}

void GlVertexArray::set_index_buffer(
    std::shared_ptr<GlIndexBuffer> index_buffer)
{
  index_buffer_ = index_buffer;

  bind();
  index_buffer_->bind();
  unbind();
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
