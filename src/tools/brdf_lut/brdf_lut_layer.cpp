#include "brdf_lut_layer.hpp"
#include "engine.hpp"
#include "gl_shader.hpp"
#include "gl_shader_storage_buffer.hpp"
#include "math.hpp"

#include <gli/save_ktx.hpp>
#include <gli/texture2d.hpp>

namespace
{

template <typename T>
gli::texture convert_lut_to_texture(const std::vector<T> &lut_data,
                                    std::uint32_t         brdf_width,
                                    std::uint32_t         brdf_height)
{
  auto lut_texture = gli::texture2d(gli::FORMAT_RG16_SFLOAT_PACK16,
                                    gli::extent2d(brdf_width, brdf_height),
                                    1);

  for (std::uint32_t y{0}; y < brdf_height; ++y)
  {
    for (std::uint32_t x{0}; x < brdf_width; ++x)
    {
      const std::uint32_t ofs{y * brdf_width + x};
      const gli::vec2     value{lut_data[ofs * 2 + 0], lut_data[ofs * 2 + 1]};
      const gli::texture::extent_type uv{x, y, 0};
      lut_texture.store<std::uint32_t>(uv, 0, gli::packHalf2x16(value));
    }
  }

  return lut_texture;
}

} // namespace

namespace dc
{

void BrdfLutLayer::init() { lut_data_.resize(buffer_size); }

void BrdfLutLayer::shutdown() {}

bool BrdfLutLayer::update(float /*delta_time*/) { return false; }

bool BrdfLutLayer::render()
{
  GlShaderStorageBuffer dst_buffer(lut_data_, GL_STATIC_READ);

  GlShader shader{"shaders/brdf_lut.comp"};
  shader.bind();

  dst_buffer.bind(0);

  compute(brdf_width, brdf_height, 1);

  // ensure all writes by the compute shader have been completed
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  const auto lut_data =
      dst_buffer.map<float>(0, lut_data_.size(), GL_MAP_READ_BIT);
  DC_ASSERT(lut_data, "Could not map lut_data");
  std::memcpy(lut_data_.data(), lut_data, lut_data_.size() * sizeof(float));
  dst_buffer.unmap();

  const auto lut_texture =
      convert_lut_to_texture(lut_data_, brdf_width, brdf_height);
  // use Pico Pixel to view https://pixelandpolygon.com/
  gli::save_ktx(lut_texture, "data/brdf_lut.ktx");

  Engine::instance()->set_close(true);

  return false;
}

bool BrdfLutLayer::on_event(const Event & /*event*/) { return false; }

} // namespace dc
