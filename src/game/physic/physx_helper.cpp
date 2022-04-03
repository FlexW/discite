#include "physx_helper.hpp"

#include <foundation/PxQuat.h>
#include <foundation/PxVec3.h>
#include <foundation/PxVec4.h>

namespace dc
{

glm::vec3 to_glm(const physx::PxVec3 &value)
{
  return {value.x, value.y, value.z};
}

glm::quat to_glm(const physx::PxQuat &value)
{
  return {value.w, value.x, value.y, value.z};
}

glm::vec3 to_glm(const physx::PxExtendedVec3 &value)
{
  return {value.x, value.y, value.z};
}

physx::PxVec3 to_physx(const glm::vec3 &value)
{
  return {value.x, value.y, value.z};
}

physx::PxVec4 to_physx(const glm::vec4 &value)
{
  return {value.x, value.y, value.z, value.w};
}

physx::PxQuat to_physx(const glm::quat &value)
{
  return {value.x, value.y, value.z, value.w};
}

physx::PxTransform to_physx_transform(const glm::vec3 &position,
                                      const glm::quat &rotation)
{
  const auto p = dc::to_physx(position);
  const auto r = dc::to_physx(rotation);
  return {p, r};
}

physx::PxTransform to_physx_transform(const glm::mat4 &transform)
{
  glm::vec3 translation{}, rotation{}, scale{};
  dc::math::decompose_transform(transform, translation, rotation, scale);

  const auto r = dc::to_physx(glm::quat{rotation});
  const auto p = dc::to_physx(translation);

  return physx::PxTransform(p, r);
}

glm::mat4 to_glm(const physx::PxTransform &transform)
{
  const auto rotation = dc::to_glm(transform.q);
  const auto position = dc::to_glm(transform.p);
  return glm::translate(glm::mat4{1.0f}, position) * glm::toMat4(rotation);
}

} // namespace dc
