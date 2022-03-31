#pragma once

#include "animation.hpp"
#include "math.hpp"

#include <string>

namespace dc
{

struct Bone
{
  std::string name_;
  int         parent_index{-1};
  glm::mat4   local_bind_pose_{1.0f};
  glm::mat4   global_inv_bind_pose_{1.0f};
};

class Skeleton
{
public:
  Skeleton() = default;
  explicit Skeleton(std::vector<Bone> bones);

  int bone_index(const std::string &name);

  void play_animation(const std::string &name);
  void stop_current_animation();

  void set_animation_endless(bool value);
  bool is_animation_endless() const;

  std::string current_animation_name() const;

  void                   compute_bone_transforms(double delta_time);
  std::vector<glm::mat4> bone_transforms() const;

  void add_animation(Animation animation);

  void save(FILE *file) const;
  void read(FILE *file);

private:
  std::vector<Bone> bones_;

  std::vector<glm::mat4> transforms_;
  std::vector<glm::mat4> transforms_identity_;

  std::vector<Animation> animations_;

  double animation_time_{0.0f};
  double animation_speed_{1.0f};
  bool   is_endless_animation_{false};
  int    active_animation_index_{-1};

  int index_of_animation_by_name(const std::string &name);

  void reset();
};

} // namespace dc
