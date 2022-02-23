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

namespace math
{

constexpr auto pi  = glm::pi<float>();
constexpr auto tau = glm::pi<float>() * 2;

bool decompose_transform(const glm::mat4 &transform,
                         glm::vec3       &translation,
                         glm::vec3       &rotation,
                         glm::vec3       &scale);
} // namespace math
