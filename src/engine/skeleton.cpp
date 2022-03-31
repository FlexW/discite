#include "skeleton.hpp"
#include "assert.hpp"
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

Skeleton::Skeleton(std::vector<Bone> bones) : bones_{std::move(bones)}
{
  reset();
}

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

void Skeleton::play_animation(const std::string &name)
{
  const auto animation_index = index_of_animation_by_name(name);

  if (animation_index == -1)
  {
    return;
  }

  animation_time_         = 0.0f;
  active_animation_index_ = animation_index;

  compute_bone_transforms(0.0f);
}

void Skeleton::stop_current_animation()
{
  animation_time_         = 0.0f;
  active_animation_index_ = -1;
}

void Skeleton::compute_bone_transforms(double delta_time)
{
  if (active_animation_index_ == -1)
  {
    return;
  }

  DC_ASSERT(0 <= active_animation_index_ &&
                static_cast<std::size_t>(active_animation_index_) <
                    animations_.size(),
            "Out of range!");

  animation_time_ += delta_time;

  const auto &animation = animations_[active_animation_index_];

  auto ticks_per_second = animation.ticks_per_second();
  ticks_per_second      = ticks_per_second != 0 ? ticks_per_second : 25.0f;
  ticks_per_second *= animation_speed_;

  const auto current_time_in_ticks = animation_time_ * ticks_per_second;

  // Is animation over?
  if (!is_endless_animation_ && current_time_in_ticks >= animation.duration())
  {
    return;
  }

  const auto animation_time =
      std::fmod(current_time_in_ticks, animation.duration());

  // Compute the bone transformations
  // Root node is special
  const auto &track = animation.track(0);
  if (track.has_value())
  {
    transforms_[0] = track.value().interpolate(animation_time);
  }
  else
  {
    transforms_[0] = bones_[0].local_bind_pose_;
  }

  for (std::size_t i = 1; i < bones_.size(); ++i)
  {
    auto       &bone  = bones_[i];
    const auto &track = animation.track(i);

    if (track.has_value())
    {
      transforms_[i] = transforms_[bone.parent_index_] *
                       track.value().interpolate(animation_time);
    }
    else
    {
      transforms_[i] = transforms_[bone.parent_index_] * bone.local_bind_pose_;
    }
  }

  for (std::size_t i = 0; i < bones_.size(); ++i)
  {
    const auto &bone = bones_[i];
    transforms_[i]   = transforms_[i] * bone.global_inv_bind_pose_;
  }
}

std::vector<glm::mat4> Skeleton::bone_transforms() const
{
  if (active_animation_index_ == -1)
  {
    return transforms_identity_;
  }
  return transforms_;
}

void Skeleton::add_animation(Animation animation)
{
  animations_.emplace_back(std::move(animation));
}

int Skeleton::index_of_animation_by_name(const std::string &name)
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

void Skeleton::save(FILE *file) const
{
  write_vector_complex(file, bones_);
  write_vector_complex(file, animations_);
  write_value(file, animation_time_);
  write_value(file, is_endless_animation_);
  write_value(file, active_animation_index_);
}

void Skeleton::read(FILE *file)
{
  read_vector_complex(file, bones_);
  read_vector_complex(file, animations_);
  read_value(file, animation_time_);
  read_value(file, is_endless_animation_);
  read_value(file, active_animation_index_);

  reset();
}

void Skeleton::reset()
{
  transforms_.clear();
  transforms_identity_.clear();

  transforms_.resize(bones_.size());

  transforms_identity_.reserve(bones_.size());
  for (std::size_t i = 0; i < bones_.size(); ++i)
  {
    transforms_identity_.push_back(glm::mat4(1.0f));
  }
}

std::string Skeleton::current_animation_name() const
{
  if (active_animation_index_ == -1)
  {
    return {};
  }

  return animations_[active_animation_index_].name();
}

void Skeleton::set_animation_endless(bool value)
{
  is_endless_animation_ = value;
}

bool Skeleton::is_animation_endless() const { return is_endless_animation_; }

} // namespace dc
