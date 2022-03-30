#pragma once

#include "math.hpp"

#include <cstddef>
#include <optional>

namespace dc
{

struct BoneRotation
{
  double    time_;
  glm::quat rotation_;
};

struct BoneTranslation
{
  double    time_;
  glm::vec3 position_;
};

struct BoneScaling
{
  double    time_;
  glm::vec3 scale_;
};

struct BoneTransform
{
  std::vector<BoneRotation>    bone_rotation_;
  std::vector<BoneTranslation> bone_translation_;
  std::vector<BoneScaling>     bone_scaling_;

  glm::mat4 interpolate(double time) const;

  void save(FILE *file) const;
  void read(FILE *file);

private:
  std::size_t find_scaling(double time) const;
  std::size_t find_rotation(double time) const;
  std::size_t find_translation(double time) const;

  glm::vec3 interpolate_scale(double time) const;
  glm::quat interpolate_rotation(double time) const;
  glm::vec3 interpolate_position(double time) const;
};

class Animation
{
public:
  Animation() = default;
  Animation(std::string                               name,
            double                                    duration,
            double                                    ticks_per_second,
            std::vector<std::optional<BoneTransform>> tracks);

  std::string name() const;
  double      duration() const;
  double      ticks_per_second() const;

  std::vector<std::optional<BoneTransform>> tracks() const;
  std::optional<BoneTransform>              track(int index) const;

  void save(FILE *file) const;
  void read(FILE *file);

private:
  std::string name_;
  double      duration_;
  double      ticks_per_second_;

  std::vector<std::optional<BoneTransform>> tracks_;
};

} // namespace dc
