#pragma once

#include "glm/ext/scalar_constants.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>

namespace math
{

constexpr auto pi  = glm::pi<float>();
constexpr auto tau = glm::pi<float>() * 2;

bool decompose_transform(const glm::mat4 &transform,
                         glm::vec3       &translation,
                         glm::vec3       &rotation,
                         glm::vec3       &scale);

struct BoundingBox
{
  glm::vec3 min_;
  glm::vec3 max_;

  BoundingBox() = default;
  BoundingBox(const glm::vec3 &min, const glm::vec3 &max)
      : min_(glm::min(min, max)),
        max_(glm::max(min, max))
  {
  }

  BoundingBox(const glm::vec3 *points, size_t numPoints)
  {
    glm::vec3 vmin(std::numeric_limits<float>::max());
    glm::vec3 vmax(std::numeric_limits<float>::lowest());

    for (size_t i = 0; i != numPoints; i++)
    {
      vmin = glm::min(vmin, points[i]);
      vmax = glm::max(vmax, points[i]);
    }
    min_ = vmin;
    max_ = vmax;
  }

  glm::vec3 getSize() const
  {
    return glm::vec3(max_[0] - min_[0], max_[1] - min_[1], max_[2] - min_[2]);
  }

  glm::vec3 getCenter() const
  {
    return 0.5f *
           glm::vec3(max_[0] + min_[0], max_[1] + min_[1], max_[2] + min_[2]);
  }

  void transform(const glm::mat4 &t)
  {
    glm::vec3 corners[] = {
        glm::vec3(min_.x, min_.y, min_.z),
        glm::vec3(min_.x, max_.y, min_.z),
        glm::vec3(min_.x, min_.y, max_.z),
        glm::vec3(min_.x, max_.y, max_.z),
        glm::vec3(max_.x, min_.y, min_.z),
        glm::vec3(max_.x, max_.y, min_.z),
        glm::vec3(max_.x, min_.y, max_.z),
        glm::vec3(max_.x, max_.y, max_.z),
    };
    for (auto &v : corners)
      v = glm::vec3(t * glm::vec4(v, 1.0f));
    *this = BoundingBox(corners, 8);
  }

  BoundingBox getTransformed(const glm::mat4 &t) const
  {
    BoundingBox b = *this;
    b.transform(t);
    return b;
  }

  void combinePoint(const glm::vec3 &p)
  {
    min_ = glm::min(min_, p);
    max_ = glm::max(max_, p);
  }
};

} // namespace math
