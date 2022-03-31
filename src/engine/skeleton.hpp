#pragma once

#include "animation.hpp"
#include "math.hpp"

#include <cstddef>
#include <string>

namespace dc
{

struct Bone
{
  std::string name_;
  int         parent_index_{-1};
  glm::mat4   local_bind_pose_{1.0f};
  glm::mat4   global_inv_bind_pose_{1.0f};

  void save(FILE *file) const;
  void read(FILE *file);
};

class Skeleton
{
public:
  Skeleton() = default;
  explicit Skeleton(std::vector<Bone> bones);

  std::vector<Bone> bones() const;
  int bone_index(const std::string &name) const;

  std::size_t bones_count() const;

  void compute_bone_transforms(std::vector<glm::mat4> &transforms,
                               int                     animation_index,
                               double                  animation_time,
                               double                  animation_speed,
                               bool                    is_endless_animation);

  void        add_animation(Animation animation);
  int         index_of_animation_by_name(const std::string &name) const;
  std::string animation_name_by_index(int animation_index) const;

  void save(FILE *file) const;
  void read(FILE *file);

private:
  std::vector<Bone> bones_;
  std::vector<Animation> animations_;
};

} // namespace dc
