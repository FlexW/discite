#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/trigonometric.hpp>

#include <vector>

namespace dc::math
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
  BoundingBox(const glm::vec3 &min, const glm::vec3 &max);

  BoundingBox(const glm::vec3 *points, size_t numPoints);

  glm::vec3 size() const;

  glm::vec3 center() const;

  void transform(const glm::mat4 &t);

  BoundingBox transformed(const glm::mat4 &t) const;

  void combine_point(const glm::vec3 &p);
};

int calc_mipmap_levels_2d(int width, int height);

} // namespace math
