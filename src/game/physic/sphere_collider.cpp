#include "sphere_collider.hpp"
#include "entity.hpp"
#include "physic_collider.hpp"
#include "physx_helper.hpp"
#include "sphere_collider_component.hpp"

#include <extensions/PxRigidActorExt.h>
#include <geometry/PxSphereGeometry.h>

namespace dc
{

SphereCollider::SphereCollider(Entity               entity,
                               physx::PxRigidActor *physx_actor,
                               const glm::vec3     &offset)
    : PhysicCollider{type},
      entity_{entity}
{
  auto &sphere_collider_component =
      entity_.component<SphereColliderComponent>();
  DC_ASSERT(!sphere_collider_component.sphere_collider_,
            "Sphere collider already set");
  sphere_collider_component.sphere_collider_ = this;

  set_physic_material(sphere_collider_component.physic_material_);

  const auto actor_scale = entity.scale();
  const auto largest_scale =
      std::max({actor_scale.x, actor_scale.y, actor_scale.z});

  const auto geometry = physx::PxSphereGeometry(
      largest_scale * sphere_collider_component.radius_);

  shape_ = physx::PxRigidActorExt::createExclusiveShape(*physx_actor,
                                                        geometry,
                                                        *get_physic_material());

  set_trigger(sphere_collider_component.is_trigger_);
  set_offset(offset);

  shape_->userData = this;
}

void SphereCollider::set_radius(float value)
{
  auto &sphere_collider_component =
      entity_.component<SphereColliderComponent>();

  const auto &actor_scale = entity_.scale();
  const auto  largest_scale =
      std::max({actor_scale.x, actor_scale.y, actor_scale.z});

  const auto geometry = physx::PxSphereGeometry(largest_scale * value);
  shape_->setGeometry(geometry);

  sphere_collider_component.radius_ = value;
}

float SphereCollider::get_radius() const
{
  return entity_.component<SphereColliderComponent>().radius_;
}

void SphereCollider::set_offset(const glm::vec3 &value)
{
  auto &sphere_collider_component =
      entity_.component<SphereColliderComponent>();
  shape_->setLocalPose(
      to_physx_transform(value + sphere_collider_component.offset_,
                         glm::vec3{0.0f}));
  sphere_collider_component.offset_ = value;
}

glm::vec3 SphereCollider::get_offset() const
{
  return entity_.component<SphereColliderComponent>().offset_;
}

void SphereCollider::set_trigger(bool value)
{
  shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !value);
  shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, value);
  entity_.component<SphereColliderComponent>().is_trigger_ = value;
}

bool SphereCollider::is_trigger() const
{
  return entity_.component<SphereColliderComponent>().is_trigger_;
}

void SphereCollider::detach_from_actor(physx::PxRigidActor &actor)
{
  actor.detachShape(*shape_);
}

} // namespace dc
