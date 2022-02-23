#pragma once

class GlVertexArray;

#include <glad/glad.h>

#include <cstdint>

void draw(const GlVertexArray &vertex_array, GLenum mode = GL_TRIANGLES);

void compute(std::uint32_t x_size, std::uint32_t y_size, std::uint32_t z_size);
