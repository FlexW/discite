#pragma once

#include "math.hpp"
#include "skeleton.hpp"

#include <memory>
#include <vector>

namespace dc
{

class AnimationState
{
public:
  AnimationState(std::shared_ptr<Skeleton> skeleton);

  void play_animation(const std::string &name);
  void stop_current_animation();

  void set_animation_endless(bool value);
  bool is_animation_endless() const;

  std::string current_animation_name() const;

  void                   compute_bone_transforms(double delta_time);
  std::vector<glm::mat4> bone_transforms() const;

  void save(FILE *file) const;
  void read(FILE *file);

private:
  std::shared_ptr<Skeleton> skeleton_;

  std::vector<glm::mat4> transforms_;
  std::vector<glm::mat4> transforms_identity_;

  double animation_time_{0.0f};
  double animation_speed_{1.0f};
  bool   is_endless_animation_{false};
  int    active_animation_index_{-1};

  void reset();
};

} // namespace dc
