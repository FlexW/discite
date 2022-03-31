#include "skeleton.hpp"
#include "assert.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "serialization.hpp"

namespace dc
{

void Bone::save(FILE *file) const
{
  write_string(file, name_);
  write_value(file, parent_index_);
  write_value(file, local_bind_pose_);
  write_value(file, global_inv_bind_pose_);
}

void Bone::read(FILE *file)
{
  read_string(file, name_);
  read_value(file, parent_index_);
  read_value(file, local_bind_pose_);
  read_value(file, global_inv_bind_pose_);
}

Skeleton::Skeleton(std::vector<Bone> bones) : bones_{std::move(bones)} {}

std::vector<Bone> Skeleton::bones() const { return bones_; }

int Skeleton::bone_index(const std::string &name) const
{
  for (std::size_t i = 0; i < bones_.size(); ++i)
  {
    if (name == bones_[i].name_)
    {
      return i;
    }
  }

  return -1;
}

std::size_t Skeleton::bones_count() const { return bones_.size(); }

void Skeleton::compute_bone_transforms(std::vector<glm::mat4> &transforms,
                                       int                     animation_index,
                                       double                  animation_time,
                                       double                  animation_speed,
                                       bool is_endless_animation)

{
  DC_ASSERT(0 <= animation_index &&
                static_cast<std::size_t>(animation_index) < animations_.size(),
            "Out of range!");

  const auto &animation = animations_[animation_index];

  auto ticks_per_second = animation.ticks_per_second();
  ticks_per_second      = ticks_per_second != 0 ? ticks_per_second : 25.0f;
  ticks_per_second *= animation_speed;

  const auto current_time_in_ticks = animation_time * ticks_per_second;

  // Is animation over?
  if (!is_endless_animation && current_time_in_ticks >= animation.duration())
  {
    return;
  }

  animation_time = std::fmod(current_time_in_ticks, animation.duration());

  // Compute the bone transformations
  // Root node is special
  const auto &track = animation.track(0);
  if (track.has_value())
  {
    transforms[0] = track.value().interpolate(animation_time);
  }
  else
  {
    transforms[0] = bones_[0].local_bind_pose_;
  }

  for (std::size_t i = 1; i < bones_.size(); ++i)
  {
    auto       &bone  = bones_[i];
    const auto &track = animation.track(i);

    if (track.has_value())
    {
      transforms[i] = transforms[bone.parent_index_] *
                      track.value().interpolate(animation_time);
    }
    else
    {
      transforms[i] = transforms[bone.parent_index_] * bone.local_bind_pose_;
    }
  }

  for (std::size_t i = 0; i < bones_.size(); ++i)
  {
    const auto &bone = bones_[i];
    transforms[i]    = transforms[i] * bone.global_inv_bind_pose_;
  }
}

void Skeleton::add_animation(Animation animation)
{
  animations_.emplace_back(std::move(animation));
}

int Skeleton::index_of_animation_by_name(const std::string &name) const
{
  for (std::size_t i = 0; i < animations_.size(); ++i)
  {
    if (animations_[i].name() == name)
    {
      return i;
    }
  }
  return -1;
}

std::string Skeleton::animation_name_by_index(int animation_index) const
{
  DC_ASSERT(0 <= animation_index &&
                static_cast<std::size_t>(animation_index) < animations_.size(),
            "Out of range!");

  return animations_[animation_index].name();
}

void Skeleton::save(FILE *file) const
{
  write_vector_complex(file, bones_);
  write_vector_complex(file, animations_);
}

void Skeleton::read(FILE *file)
{
  read_vector_complex(file, bones_);
  read_vector_complex(file, animations_);
}

} // namespace dc
