#pragma once

#include <cstddef>
#include <vector>

namespace dc
{

enum class VertexBufferLayoutElementType
{
  Int,
  Float,
};

struct VertexBufferLayoutElement
{
  std::size_t                   size;
  unsigned                      count;
  VertexBufferLayoutElementType type;
};

class VertexBufferLayout
{
public:
  void push_float(unsigned count);
  void push_int(unsigned count);

  std::vector<VertexBufferLayoutElement> elements() const;

  std::size_t size() const;

private:
  std::size_t                            size_ = 0;
  std::vector<VertexBufferLayoutElement> elements_;
};

} // namespace dc
