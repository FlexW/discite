#pragma once

#include "gl.hpp"
#include "graphic/renderer.hpp"
#include "graphic/texture.hpp"
#include "graphic/vertex_buffer_layout.hpp"

namespace dc
{

GLenum to_gl(TextureFormat format);

GLenum to_gl(TextureSizedFormat format);

GLenum to_gl(TextureDataType type);

GLenum to_gl(TextureWrapMode mode);

GLenum to_gl(TextureFilterMode mode);

GLenum to_gl(VertexBufferLayoutElementType type);

GLenum to_gl(DrawMode draw_mode);

} // namespace dc
