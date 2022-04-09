#pragma once

#include "entity.hpp"
#include "physic_collider.hpp"

#include <PxActor.h>
#include <PxRigidActor.h>
#include <PxShape.h>

namespace dc
{

class SphereCollider : public PhysicCollider
{
public:
  static constexpr PhysicColliderType type{PhysicColliderType::Sphere};

  SphereCollider(Entity               entity,
                 physx::PxRigidActor *physx_actor,
                 const glm::vec3     &offset);

  void  set_radius(float value);
  float get_radius() const;

  void      set_offset(const glm::vec3 &value) override;
  glm::vec3 get_offset() const override;

  void set_trigger(bool value) override;
  bool is_trigger() const override;

  void detach_from_actor(physx::PxRigidActor &actor) override;

private:
  Entity entity_;

  physx::PxShape *shape_{};
};

} // namespace dc
