#pragma once

namespace dc
{

class IndexBuffer
{
public:
  virtual ~IndexBuffer() = default;

  virtual unsigned count() const = 0;
};

} // namespace dc
