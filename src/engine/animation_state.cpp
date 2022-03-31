#include "animation_state.hpp"
#include "assert.hpp"
#include "serialization.hpp"

namespace dc
{

AnimationState::AnimationState(std::shared_ptr<Skeleton> skeleton)
    : skeleton_{skeleton}
{
  DC_ASSERT(skeleton_, "Skeleton needs to be set");
  reset();
}

void AnimationState::play_animation(const std::string &name)
{
  const auto animation_index = skeleton_->index_of_animation_by_name(name);

  if (animation_index == -1)
  {
    return;
  }

  animation_time_         = 0.0f;
  active_animation_index_ = animation_index;

  compute_bone_transforms(0.0f);
}

void AnimationState::stop_current_animation()
{
  animation_time_         = 0.0f;
  active_animation_index_ = -1;
}

void AnimationState::set_animation_endless(bool value)
{
  is_endless_animation_ = value;
}

bool AnimationState::is_animation_endless() const
{
  return is_endless_animation_;
}

void AnimationState::compute_bone_transforms(double delta_time)
{
  if (active_animation_index_ == -1)
  {
    return;
  }

  animation_time_ += delta_time;

  skeleton_->compute_bone_transforms(transforms_,
                                     active_animation_index_,
                                     animation_time_,
                                     animation_speed_,
                                     is_endless_animation_);
}

std::vector<glm::mat4> AnimationState::bone_transforms() const
{
  if (active_animation_index_ == -1)
  {
    return transforms_identity_;
  }
  return transforms_;
}

void AnimationState::reset()
{
  const auto bones = skeleton_->bones();

  transforms_.clear();
  transforms_identity_.clear();

  transforms_.resize(bones.size());

  transforms_identity_.reserve(bones.size());
  for (std::size_t i = 0; i < bones.size(); ++i)
  {
    transforms_identity_.push_back(glm::mat4(1.0f));
  }
}

void AnimationState::save(FILE *file) const
{
  write_value(file, animation_time_);
  write_value(file, is_endless_animation_);
  write_value(file, active_animation_index_);
}

void AnimationState::read(FILE *file)
{
  read_value(file, animation_time_);
  read_value(file, is_endless_animation_);
  read_value(file, active_animation_index_);

  reset();
  compute_bone_transforms(0);
}

std::string AnimationState::current_animation_name() const
{
  if (active_animation_index_ == -1)
  {
    return {};
  }
  return skeleton_->animation_name_by_index(active_animation_index_);
}

} // namespace dc
