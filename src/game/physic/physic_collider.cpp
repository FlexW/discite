#include "physic_collider.hpp"
#include "physx_sdk.hpp"

#include <PxMaterial.h>

namespace dc
{

PhysicCollider::PhysicCollider(PhysicColliderType type)
    : physic_collider_type_{type}
{
}

PhysicCollider::~PhysicCollider()
{
  if (material_)
  {
    material_->release();
    material_ = nullptr;
  }
}

void PhysicCollider::set_physic_material(const PhysicMaterial &value)
{
  if (material_)
  {
    material_->release();
    material_ = nullptr;
  }

  material_ = PhysXSdk::get_instance()->get_physics()->createMaterial(
      value.static_friction_,
      value.dynamic_friction_,
      value.bounciness_);
}

physx::PxMaterial *PhysicCollider::get_physic_material() const
{
  return material_;
}

PhysicColliderType PhysicCollider::get_physic_collider_type() const
{
  return physic_collider_type_;
}

bool PhysicCollider::is_valid() const { return true; }

} // namespace dc
