#pragma once

#include "entity.hpp"
#include "mesh_collider_data.hpp"
#include "physic_collider.hpp"

#include <PxRigidActor.h>
#include <PxShape.h>

#include <vector>

namespace dc
{

class MeshCollider : public PhysicCollider
{
public:
  static constexpr PhysicColliderType type{PhysicColliderType::TriangleMesh};

  MeshCollider(Entity               entity,
               physx::PxRigidActor *physx_actor,
               MeshColliderType     mesh_collider_type);

  void set_trigger(bool value) override;
  bool is_trigger() const override;

  void      set_offset(const glm::vec3 &offset) override;
  glm::vec3 get_offset() const override;

  void detach_from_actor(physx::PxRigidActor &actor) override;

private:
  Entity entity_;

  std::vector<physx::PxShape *> shapes_;
};

} // namespace dc
