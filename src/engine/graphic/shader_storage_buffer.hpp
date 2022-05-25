#pragma once

#include <cstddef>

namespace dc
{

enum class ShaderStorageAccessMode
{
  Unspecifed,
  MapWrite,
  MapRead,
  MapReadWrite,
};

class ShaderStorageBuffer
{
public:
  virtual ~ShaderStorageBuffer() = default;

  template <typename T>
  T *map(std::size_t             offset,
         std::size_t             count,
         ShaderStorageAccessMode access_mode =
             ShaderStorageAccessMode::MapReadWrite)
  {
    return static_cast<T *>(map(offset, count * sizeof(T), access_mode));
  }

  virtual void unmap() = 0;

protected:
  virtual void *map(std::size_t             offset,
                    std::size_t             size,
                    ShaderStorageAccessMode access_mode) = 0;
};

} // namespace dc
