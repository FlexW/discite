#pragma once

#include "math.hpp"

#include <PxPhysics.h>
#include <characterkinematic/PxExtended.h>

namespace dc
{

glm::vec3 to_glm(const physx::PxVec3 &value);

glm::quat to_glm(const physx::PxQuat &value);

glm::vec3 to_glm(const physx::PxExtendedVec3 &value);

physx::PxVec3 to_physx(const glm::vec3 &value);

physx::PxVec4 to_physx(const glm::vec4 &value);

physx::PxQuat to_physx(const glm::quat &value);

template <typename T> static void px_release(T *obj)
{
  if (obj)
  {
    obj->release();
  }
}

physx::PxTransform to_physx_transform(const glm::vec3 &position,
                                      const glm::quat &rotation);

physx::PxTransform to_physx_transform(const glm::mat4 &transform);

glm::mat4 to_glm(const physx::PxTransform &transform);

} // namespace dc
