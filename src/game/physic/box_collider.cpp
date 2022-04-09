#include "box_collider.hpp"
#include "box_collider_component.hpp"
#include "entity.hpp"
#include "physic_actor.hpp"
#include "physic_collider.hpp"
#include "physx_helper.hpp"

#include <PxShape.h>
#include <extensions/PxRigidActorExt.h>
#include <extensions/PxRigidBodyExt.h>
#include <geometry/PxBoxGeometry.h>
#include <geometry/PxGeometry.h>

namespace dc
{

BoxCollider::BoxCollider(Entity               entity,
                         physx::PxRigidActor *physx_actor,
                         const glm::vec3     &offset)
    : PhysicCollider{type},
      entity_{entity}
{
  auto &box_collider_component = entity_.component<BoxColliderComponent>();
  DC_ASSERT(!box_collider_component.box_collider_, "Box collider already set");
  box_collider_component.box_collider_ = this;

  set_physic_material(box_collider_component.physic_material_);

  const auto collider_size = entity_.scale() * box_collider_component.size_;
  const auto geometry =
      physx::PxBoxGeometry(collider_size.x, collider_size.y, collider_size.z);

  shape_ = physx::PxRigidActorExt::createExclusiveShape(*physx_actor,
                                                        geometry,
                                                        *get_physic_material());

  set_trigger(box_collider_component.is_trigger_);
  set_offset(offset);

  shape_->userData = this;
}

void BoxCollider::set_size(const glm::vec3 &size) {
  const auto collider_size = entity_.scale() * size;

  const physx::PxBoxGeometry geometry{collider_size.x,
                                      collider_size.y,
                                      collider_size.z};

  shape_->setGeometry(geometry);

  entity_.component<BoxColliderComponent>().size_ = size;
}

glm::vec3 BoxCollider::get_size() const
{
  return entity_.component<BoxColliderComponent>().size_;
}

void BoxCollider::set_offset(const glm::vec3 &value)
{
  auto &box_collider_component = entity_.component<BoxColliderComponent>();
  shape_->setLocalPose(
      to_physx_transform(value + box_collider_component.offset_,
                         glm::vec3{0.0f}));
  box_collider_component.offset_ = value;
}

glm::vec3 BoxCollider::get_offset() const
{
  return entity_.component<BoxColliderComponent>().offset_;
}

void BoxCollider::set_trigger(bool value)
{

  shape_->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !value);
  shape_->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, value);
  entity_.component<BoxColliderComponent>().is_trigger_ = value;
}

bool BoxCollider::is_trigger() const
{
  return entity_.component<BoxColliderComponent>().is_trigger_;
}

void BoxCollider::detach_from_actor(physx::PxRigidActor &actor)
{
  actor.detachShape(*shape_);
}

} // namespace dc
