#pragma once

#include <glad/glad.h>

#include <cstdint>

namespace dc
{

class GlVertexArray;

void draw(const GlVertexArray &vertex_array,
          GLenum               mode  = GL_TRIANGLES,
          long                 count = -1);

void compute(std::uint32_t x_size, std::uint32_t y_size, std::uint32_t z_size);

} // namespace dc
