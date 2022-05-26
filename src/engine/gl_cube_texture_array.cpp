#include "gl_cube_texture_array.hpp"
#include "assert.hpp"
#include "math.hpp"

namespace dc
{

GlCubeTextureArray::GlCubeTextureArray(const GlCubeTextureArrayConfig &config)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &id_);

    DC_ASSERT(config.count > 0, "Texture array data too small");

    const auto levels =
        config.is_generate_mipmap
            ? glm::log2(glm::max(config.width, config.height)) + 1
            : 1;

    // reserve space for textures
    glTextureStorage3D(id_,
                       levels,
                       config.sized_format,
                       config.width,
                       config.height,
                       config.count * 6);

    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, config.min_filter);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, config.mag_filter);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, config.wrap_s);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, config.wrap_t);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, config.wrap_r);

    if (config.is_generate_mipmap)
    {
        glGenerateTextureMipmap(id_);
    }
}

GlCubeTextureArray::~GlCubeTextureArray()
{
    glDeleteTextures(1, &id_);
}

GLuint GlCubeTextureArray::id() const
{
    return id_;
}

void GlCubeTextureArray::bind_unit(int unit) const
{
    glBindTextureUnit(unit, id_);
}

} // namespace dc
