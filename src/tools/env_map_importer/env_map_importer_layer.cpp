#include "env_map_importer_layer.hpp"
#include "asset.hpp"
#include "cmd_args_parser.hpp"
#include "defer.hpp"
#include "engine.hpp"
#include "entt/entity/helper.hpp"
#include "gl_shader.hpp"
#include "gl_shader_storage_buffer.hpp"
#include "importer.hpp"
#include "log.hpp"
#include "math.hpp"
#include "serialization.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <stdexcept>

namespace
{

using namespace dc;

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

std::vector<std::uint8_t> process_cubemap(const std::string &filename)
{
  int        w{}, h{}, comp{};
  const auto img = stbi_loadf(filename.c_str(), &w, &h, &comp, 3);

  if (!img)
  {
    throw std::runtime_error("Failed to load [%s] texture " + filename);
    fflush(stdout);
    return {};
  }

  const int dstW = 256;
  const int dstH = 128;

  std::vector<glm::vec3> out(dstW * dstH);

  convolve_diffuse((glm::vec3 *)img, w, h, dstW, dstH, out.data(), num_points);

  stbi_image_free((void *)img);

  std::vector<std::uint8_t> out_data;
  stbi_write_hdr_to_func(
      [](void *context, void *data, int size)
      {
        const auto out_data = static_cast<std::vector<std::uint8_t> *>(context);
        const auto old_size = out_data->size();
        out_data->resize(old_size + size);
        std::memcpy(out_data->data() + old_size, data, size);
      },
      reinterpret_cast<void *>(&out_data),
      dstW,
      dstH,
      3,
      reinterpret_cast<float *>(out.data()));
  return out_data;
}

} // namespace

namespace dc
{

void EnvMapImporter::add_cmd_line_args(ArgsParser &args_parser)
{
  ArgsParser::Option hdr_map_option;
  hdr_map_option.name_        = "hdr-map";
  hdr_map_option.description_ = "File path of HDR map to import";
  hdr_map_option.type_        = ArgsParser::OptionType::Value;
  hdr_map_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(hdr_map_option);

  ArgsParser::Option name_option;
  name_option.name_        = "name";
  name_option.description_ = "Name of the imported environment map";
  name_option.type_        = ArgsParser::OptionType::Value;
  name_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(name_option);
}

void EnvMapImporter::eval_cmd_line_args(ArgsParser &args_parser)
{

  file_path_ = args_parser.value_as_string("hdr-map").value();
  name_      = args_parser.value_as_string("name").value();
}

void EnvMapImporter::init() {}

void EnvMapImporter::shutdown() {}

void EnvMapImporter::update(float /*delta_time*/)
{
  const auto base_directory = Engine::instance()->base_directory();

  auto env_irr_map_data = process_cubemap(file_path_.string());

  {
    const auto file = std::fopen(file_path_.string().c_str(), "rb");
    if (!file)
    {
      throw std::runtime_error("Could not open file " + file_path_.string());
    }
    defer(std::fclose(file));

    std::fseek(file, 0, SEEK_END);
    const auto size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    std::vector<std::uint8_t> hdr_map_data(size);
    std::fread(hdr_map_data.data(), sizeof(std::uint8_t), size, file);

    const auto base_directory = Engine::instance()->base_directory();
    std::filesystem::create_directories(base_directory / "envs");

    AssetDescription          asset_description{};
    EnvironmentMapDescription env_map_description{};
    env_map_description.env_map_data_     = std::move(hdr_map_data);
    env_map_description.env_irr_map_data_ = std::move(env_irr_map_data);
    env_map_description.save(base_directory / "envs" /
                                 sanitize_file_path(name_ + ".dcenv"),
                             asset_description);
  }

  Engine::instance()->set_close(true);
}

void EnvMapImporter::render() {}

bool EnvMapImporter::on_event(const Event & /*event*/) { return false; }

} // namespace dc
