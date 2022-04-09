#pragma once

#include "math.hpp"
#include "physic_material.hpp"

#include <PxMaterial.h>
#include <PxRigidActor.h>

namespace dc
{

enum class PhysicColliderType
{
  Box,
  Sphere,
  Capsule,
  ConvexMesh,
  TriangleMesh,
};

class PhysicCollider
{
protected:
  PhysicCollider(PhysicColliderType type);

public:
  virtual ~PhysicCollider();

  void           set_physic_material(const PhysicMaterial &value);
  physx::PxMaterial *get_physic_material() const;

  PhysicColliderType get_physic_collider_type() const;

  bool is_valid() const;

  virtual void      set_offset(const glm::vec3 &offset) = 0;
  virtual glm::vec3 get_offset() const                  = 0;

  virtual void set_trigger(bool value) = 0;
  virtual bool is_trigger() const      = 0;

  virtual void detach_from_actor(physx::PxRigidActor &actor) = 0;

private:
  PhysicColliderType physic_collider_type_;
  physx::PxMaterial *material_{};
};

} // namespace dc
