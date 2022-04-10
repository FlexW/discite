#pragma once

#include "audio_asset.hpp"
#include "audio_source.hpp"
#include "math.hpp"

#include <memory>

namespace dc
{

struct AudioSourceComponent
{
  float     pitch_{1.0f};
  float     gain_{1.0f};
  glm::vec3 velocity_{glm::vec3{0.0f}};

  bool looping_{false};

  std::shared_ptr<AudioAsset>  audio_asset_{};
  std::shared_ptr<AudioSource> audio_source_{};
};

} // namespace dc
