#include "vertex_buffer_layout.hpp"

namespace dc
{

void VertexBufferLayout::push_float(unsigned count)
{
  VertexBufferLayoutElement element{};
  element.type  = VertexBufferLayoutElementType::Float;
  element.count = count;
  element.size  = element.count * sizeof(float);
  elements_.push_back(element);

  size_ += element.size;
}

void VertexBufferLayout::push_int(unsigned count)
{
  VertexBufferLayoutElement element{};
  element.type   = VertexBufferLayoutElementType::Int;
  element.count  = count;
  element.size   = element.count * sizeof(int);
  elements_.push_back(element);

  size_ += element.size;
}

std::vector<VertexBufferLayoutElement> VertexBufferLayout::elements() const
{
  return elements_;
}

std::size_t VertexBufferLayout::size() const { return size_; }

} // namespace dc
