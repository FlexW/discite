#include "animation.hpp"
#include "assert.hpp"
#include "serialization.hpp"

#include <cstddef>

namespace dc
{

glm::mat4 BoneTransform::interpolate(double time) const
{
  const auto position = interpolate_position(time);
  const auto rotation = interpolate_rotation(time);
  const auto scale    = interpolate_scale(time);

  const auto rotation_mat = glm::toMat4(rotation);
  const auto scale_mat    = glm::scale(glm::mat4{1.0f}, scale);
  const auto translate_mat = glm::translate(glm::mat4{1.0f}, position);

  return translate_mat * rotation_mat * scale_mat;
}

std::size_t BoneTransform::find_scaling(double time) const
{
  DC_ASSERT(bone_scaling_.size() > 0, "Out of range!");

  for (std::size_t i = 0; i < bone_scaling_.size() - 1; ++i)
  {
    if (time < bone_scaling_[i + 1].time_)
    {
      return i;
    }
  }

  DC_FAIL("No scaling found");
}

std::size_t BoneTransform::find_rotation(double time) const
{
  DC_ASSERT(bone_rotation_.size() > 0, "Out of range!");

  for (std::size_t i = 0; i < bone_rotation_.size() - 1; ++i)
  {
    if (time < bone_rotation_[i + 1].time_)
    {
      return i;
    }
  }

  DC_FAIL("No rotation found");
}

std::size_t BoneTransform::find_translation(double time) const
{
  DC_ASSERT(bone_translation_.size() > 0, "Out of range!");

  for (std::size_t i = 0; i < bone_translation_.size() - 1; ++i)
  {
    if (time < bone_translation_[i + 1].time_)
    {
      return i;
    }
  }

  DC_FAIL("No translation found");
}

glm::vec3 BoneTransform::interpolate_scale(double time) const
{
  if (bone_scaling_.size() == 1)
  {
    return bone_scaling_[0].scale_;
  }

  const auto scaling_index      = find_scaling(time);
  const auto scaling_index_next = scaling_index + 1;
  DC_ASSERT(scaling_index_next < bone_scaling_.size(), "Out of range!");

  const auto delta_time = bone_scaling_[scaling_index_next].time_ -
                          bone_scaling_[scaling_index].time_;
  auto factor = (time - bone_scaling_[scaling_index].time_) / delta_time;

  // maybe the animation data does not start at time zero
  if (factor <= 0.0f)
  {
    factor = 0.0f;
  }
  DC_ASSERT(factor >= 0.0f && factor <= 1.0f, "Out of range!");

  const auto start = bone_scaling_[scaling_index].scale_;
  const auto end   = bone_scaling_[scaling_index_next].scale_;
  const auto delta = end - start;
  return start +
         glm::vec3(factor * delta.x, factor * delta.y, factor * delta.z);
}

glm::quat BoneTransform::interpolate_rotation(double time) const
{
  if (bone_rotation_.size() == 1)
  {
    return bone_rotation_[0].rotation_;
  }

  const auto rotation_index      = find_rotation(time);
  const auto rotation_index_next = rotation_index + 1;
  DC_ASSERT(rotation_index_next < bone_rotation_.size(), "Out of range!");

  const auto delta_time = bone_rotation_[rotation_index_next].time_ -
                          bone_rotation_[rotation_index].time_;
  auto factor = static_cast<float>(
      (time - bone_rotation_[rotation_index].time_) / delta_time);

  // Maybe the animation data does not start at time zero
  if (factor <= 0.0f)
  {
    factor = 0.0f;
  }
  DC_ASSERT(factor >= 0.0f && factor <= 1.0f, "Out of range!");

  const auto start = bone_rotation_[rotation_index].rotation_;
  const auto end   = bone_rotation_[rotation_index_next].rotation_;

  return glm::normalize(glm::slerp(start, end, factor));
}

glm::vec3 BoneTransform::interpolate_position(double time) const
{
  if (bone_translation_.size() == 1)
  {
    return bone_translation_[0].position_;
  }

  const auto translation_index      = find_translation(time);
  const auto translation_index_next = translation_index + 1;
  DC_ASSERT(translation_index_next < bone_translation_.size(), "Out of range!");

  const auto delta_time = bone_translation_[translation_index_next].time_ -
                          bone_translation_[translation_index].time_;
  auto factor =
      (time - bone_translation_[translation_index].time_) / delta_time;

  // Maybe the animation data does not start at time zero
  if (factor <= 0.0f)
  {
    factor = 0.0f;
  }
  DC_ASSERT(factor >= 0.0f && factor <= 1.0f, "Out of range!");

  const auto start = bone_translation_[translation_index].position_;
  const auto end   = bone_translation_[translation_index_next].position_;
  const auto delta = end - start;
  return start +
         glm::vec3(factor * delta.x, factor * delta.y, factor * delta.z);
}

void BoneTransform::save(FILE *file) const
{
  write_vector(file, bone_rotation_);
  write_vector(file, bone_translation_);
  write_vector(file, bone_scaling_);
}

void BoneTransform::read(FILE *file)
{
  read_vector(file, bone_rotation_);
  read_vector(file, bone_translation_);
  read_vector(file, bone_scaling_);
}

Animation::Animation(std::string                               name,
                     double                                    duration,
                     double                                    ticks_per_second,
                     std::vector<std::optional<BoneTransform>> tracks)
    : name_{std::move(name)},
      duration_{duration},
      ticks_per_second_{ticks_per_second},
      tracks_{std::move(tracks)}
{
}

std::string Animation::name() const { return name_; }

double Animation::duration() const { return duration_; }

double Animation::ticks_per_second() const { return ticks_per_second_; }

std::vector<std::optional<BoneTransform>> Animation::tracks() const
{
  return tracks_;
}

std::optional<BoneTransform> Animation::track(int index) const
{
  DC_ASSERT(0 <= index && static_cast<std::size_t>(index) < tracks_.size(),
            "Index is out of range!");
  return tracks_[index];
}

void Animation::save(FILE *file) const
{
  write_string(file, name_);
  write_value(file, duration_);
  write_value(file, ticks_per_second_);

  write_value(file, tracks_.size());
  for (std::size_t i = 0; i < tracks_.size(); ++i)
  {
    const auto &track     = tracks_[i];
    const auto &has_value = track.has_value();
    write_value(file, has_value);
    if (has_value)
    {
      track.value().save(file);
    }
  }
}

void Animation::read(FILE *file)
{
  read_string(file, name_);
  read_value(file, duration_);
  read_value(file, ticks_per_second_);

  std::size_t tracks_size{0};
  read_value(file, tracks_size);
  tracks_.resize(tracks_size);
  for (std::size_t i = 0; i < tracks_.size(); ++i)
  {
    bool has_value{false};
    read_value(file, has_value);
    if (has_value)
    {
      BoneTransform bone_transform{};
      bone_transform.read(file);
      tracks_[i] = std::move(bone_transform);
    }
  }
}

} // namespace dc
