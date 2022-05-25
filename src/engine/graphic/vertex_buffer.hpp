#pragma once

#include "vertex_buffer_layout.hpp"

#include <cstddef>

namespace dc
{

enum class VertexBufferUsage
{
  Unspecified,
  MapWrite,
  MapRead,
  MapReadWrite,
  MapPersistentWrite,
  MapPersistentRead,
  MapPersistentReadWrite,
};

class VertexBuffer
{
public:
  virtual ~VertexBuffer() = default;

  template <typename T>
  void write(const std::vector<T> &data, std::size_t offset)
  {
    const auto size = data.size() * sizeof(T);
    write(data.data(), size, offset);
  }

  virtual void
  write(const void *data, std::size_t size, std::size_t offset) = 0;

  virtual VertexBufferLayout layout() const = 0;
  virtual unsigned           count() const  = 0;
  virtual std::size_t        size() const   = 0;
};

} // namespace dc
