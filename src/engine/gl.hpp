#pragma once

class GlVertexArray;

#include <glad/glad.h>

void draw(const GlVertexArray &vertex_array, GLenum mode = GL_TRIANGLES);
