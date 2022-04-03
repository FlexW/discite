#include "capsule_collider.hpp"
#include "capsule_collider_component.hpp"
#include "entity.hpp"
#include "physic_collider.hpp"
#include "physx_helper.hpp"

#include <extensions/PxRigidActorExt.h>
#include <geometry/PxCapsuleGeometry.h>

namespace dc
{

CapsuleCollider::CapsuleCollider(Entity               entity,
                                 physx::PxRigidActor *physx_actor,
                                 const glm::vec3     &offset)
    : PhysicCollider{type},
      entity_{entity}
{
  const auto &capsule_collider_component =
      entity_.component<CapsuleColliderComponent>();

  set_physic_material(capsule_collider_component.physic_material_);

  const auto actor_scale  = entity.scale();
  const auto radius_scale = glm::max(actor_scale.x, actor_scale.z);

  const auto geometry = physx::PxCapsuleGeometry(
      capsule_collider_component.radius_ * radius_scale,
      (capsule_collider_component.height_ / 2.0f) * actor_scale.y);

  shape_ = physx::PxRigidActorExt::createExclusiveShape(*physx_actor,
                                                        geometry,
                                                        *get_physic_material());

  set_trigger(capsule_collider_component.is_trigger_);
  shape_->setLocalPose(
      to_physx_transform(offset + capsule_collider_component.offset_,
                         glm::vec3{0.0f, 0.0f, physx::PxHalfPi}));

  shape_->userData = this;
}

void CapsuleCollider::set_radius(float value)
{
  const auto actor_scale  = entity_.scale();
  const auto radius_scale = glm::max(actor_scale.x, actor_scale.z);

  physx::PxCapsuleGeometry old_geometry;
  shape_->getCapsuleGeometry(old_geometry);

  const auto geometry =
      physx::PxCapsuleGeometry(radius_scale * value, old_geometry.halfHeight);
  shape_->setGeometry(geometry);

  entity_.component<CapsuleColliderComponent>().radius_ = value;
}

float CapsuleCollider::get_radius() const
{
  return entity_.component<CapsuleColliderComponent>().radius_;
}

void CapsuleCollider::set_height(float value)
{
  const auto actor_scale  = entity_.scale();

  physx::PxCapsuleGeometry old_geometry;
  shape_->getCapsuleGeometry(old_geometry);

  const auto geometry =
      physx::PxCapsuleGeometry(old_geometry.radius,
                               (value / 2.0f) * actor_scale.y);
  shape_->setGeometry(geometry);

  entity_.component<CapsuleColliderComponent>().radius_ = value;
}

float CapsuleCollider::get_height() const
{
  return entity_.component<CapsuleColliderComponent>().height_;
}

void CapsuleCollider::set_offset(const glm::vec3 &value)
{
  auto &component = entity_.component<CapsuleColliderComponent>();
  shape_->setLocalPose(
      to_physx_transform(value + component.offset_, glm::vec3{0.0f}));
  component.offset_ = value;
}

glm::vec3 CapsuleCollider::get_offset() const
{
  return entity_.component<CapsuleColliderComponent>().offset_;
}

void CapsuleCollider::set_trigger(bool value)
{
  shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !value);
  shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, value);
  entity_.component<CapsuleColliderComponent>().is_trigger_ = value;
}

bool CapsuleCollider::is_trigger() const
{
  return entity_.component<CapsuleColliderComponent>().is_trigger_;
}

void CapsuleCollider::detach_from_actor(physx::PxRigidActor &actor)
{
  actor.detachShape(*shape_);
}

std::string CapsuleCollider::get_collider_name() const
{
  return "CapsuleCollider";
}

} // namespace dc
