#include "irr_map_layer.hpp"
#include "engine.hpp"
#include "gl_shader.hpp"
#include "gl_shader_storage_buffer.hpp"
#include "math.hpp"

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

namespace
{

constexpr auto num_points{1024};

/// From Henry J. Warren's "Hacker's Delight"
float radical_inverse_vdc(uint32_t bits)
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return static_cast<float>(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

/// From http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

glm::vec2 hammersley2d(uint32_t i, uint32_t N)
{
  return glm::vec2(float(i) / float(N), radical_inverse_vdc(i));
}

void convolve_diffuse(const glm::vec3 *data,
                      int              srcW,
                      int              srcH,
                      int              dstW,
                      int              dstH,
                      glm::vec3       *output,
                      int              numMonteCarloSamples)
{
  // only equirectangular maps are supported
  assert(srcW == 2 * srcH);

  if (srcW != 2 * srcH)
    return;

  std::vector<glm::vec3> tmp(dstW * dstH);

  stbir_resize_float_generic(reinterpret_cast<const float *>(data),
                             srcW,
                             srcH,
                             0,
                             reinterpret_cast<float *>(tmp.data()),
                             dstW,
                             dstH,
                             0,
                             3,
                             STBIR_ALPHA_CHANNEL_NONE,
                             0,
                             STBIR_EDGE_CLAMP,
                             STBIR_FILTER_CUBICBSPLINE,
                             STBIR_COLORSPACE_LINEAR,
                             nullptr);

  const glm::vec3 *scratch = tmp.data();
  srcW                     = dstW;
  srcH                     = dstH;

  for (int y = 0; y != dstH; y++)
  {
    printf("Line %i...\n", y);
    const float theta1 = float(y) / float(dstH) * math::pi;
    for (int x = 0; x != dstW; x++)
    {
      const float     phi1   = float(x) / float(dstW) * math::tau;
      const glm::vec3 V1     = glm::vec3(sin(theta1) * cos(phi1),
                                     sin(theta1) * sin(phi1),
                                     cos(theta1));
      glm::vec3       color  = glm::vec3(0.0f);
      float           weight = 0.0f;
      for (int i = 0; i != numMonteCarloSamples; i++)
      {
        const glm::vec2 h      = hammersley2d(i, numMonteCarloSamples);
        const int       x1     = int(floor(h.x * srcW));
        const int       y1     = int(floor(h.y * srcH));
        const float     theta2 = float(y1) / float(srcH) * math::pi;
        const float     phi2   = float(x1) / float(srcW) * math::tau;
        const glm::vec3 V2     = glm::vec3(sin(theta2) * cos(phi2),
                                       sin(theta2) * sin(phi2),
                                       cos(theta2));
        const float     D      = std::max(0.0f, glm::dot(V1, V2));
        if (D > 0.01f)
        {
          color += scratch[y1 * srcW + x1] * D;
          weight += D;
        }
      }
      output[y * dstW + x] = color / weight;
    }
  }
}

void process_cubemap(const char *filename, const char *outFilename)
{
  int          w, h, comp;
  const float *img = stbi_loadf(filename, &w, &h, &comp, 3);

  if (!img)
  {
    printf("Failed to load [%s] texture\n", filename);
    fflush(stdout);
    return;
  }

  const int dstW = 256;
  const int dstH = 128;

  std::vector<glm::vec3> out(dstW * dstH);

  convolve_diffuse((glm::vec3 *)img, w, h, dstW, dstH, out.data(), num_points);

  stbi_image_free((void *)img);
  stbi_write_hdr(outFilename, dstW, dstH, 3, (float *)out.data());
}

} // namespace

void IrrMapLayer::init() {}

void IrrMapLayer::shutdown() {}

void IrrMapLayer::update(float /*delta_time*/)
{
  process_cubemap("data/piazza_bologni_1k.hdr",
                  "data/piazza_bologni_1k_irradiance.hdr");
  Engine::instance()->set_close(true);
}

void IrrMapLayer::render() {}

bool IrrMapLayer::on_event(const Event & /*event*/) { return false; }
