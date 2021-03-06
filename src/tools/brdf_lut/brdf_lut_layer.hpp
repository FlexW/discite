#pragma once

#include "layer.hpp"

#include <array>

namespace dc
{

class BrdfLutLayer : public Layer
{
public:
  void init() override;
  void shutdown() override;

  bool update(float delta_time) override;
  bool render() override;

  bool on_event(const Event &event) override;

private:
  static constexpr int brdf_width{256};
  static constexpr int brdf_height{256};

  static constexpr std::uint32_t buffer_size{2 * brdf_width * brdf_height};
  std::vector<float>             lut_data_;
};

} // namespace dc
