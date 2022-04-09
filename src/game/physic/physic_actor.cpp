#include "physic_actor.hpp"
#include "assert.hpp"
#include "box_collider_component.hpp"
#include "capsule_collider_component.hpp"
#include "entity.hpp"
#include "log.hpp"
#include "math.hpp"
#include "mesh_collider_component.hpp"
#include "physic/box_collider.hpp"
#include "physic/capsule_collider.hpp"
#include "physic/mesh_collider.hpp"
#include "physic/mesh_collider_data.hpp"
#include "physic/sphere_collider.hpp"
#include "physic_settings.hpp"
#include "physx_helper.hpp"
#include "physx_sdk.hpp"
#include "profiling.hpp"
#include "rigid_body_component.hpp"
#include "sphere_collider_component.hpp"
#include "transform_component.hpp"

#include <PxForceMode.h>
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <extensions/PxRigidBodyExt.h>
#include <foundation/PxTransform.h>
#include <memory>

namespace
{

physx::PxForceMode::Enum to_physx(dc::ForceMode force_mode)
{
  switch (force_mode)
  {
  case dc::ForceMode::Force:
    return physx::PxForceMode::eFORCE;
  case dc::ForceMode::Impulse:
    return physx::PxForceMode::eIMPULSE;
  case dc::ForceMode::VelocityChange:
    return physx::PxForceMode::eVELOCITY_CHANGE;
  case dc::ForceMode::Acceleration:
    return physx::PxForceMode::eACCELERATION;
  }
  return physx::PxForceMode::eFORCE;
}

} // namespace

namespace dc
{

PhysicActor::PhysicActor(Entity entity) : entity_{entity}
{
  DC_ASSERT(entity.has_component<RigidBodyComponent>(),
            "Entity has not rigid body component");

  const auto &rigid_body_component = entity.component<RigidBodyComponent>();
  const auto &settings             = default_physic_settings();
  const auto &sdk                  = PhysXSdk::get_instance()->get_physics();

  rigid_body_type_ = rigid_body_component.body_type_;
  if (rigid_body_type_ == RigidBodyType::Static)
  {
    rigid_actor_ =
        sdk->createRigidStatic(to_physx_transform(entity.transform_matrix()));
  }
  else
  {
    const auto rigid_actor =
        sdk->createRigidDynamic(to_physx_transform(entity.transform_matrix()));
    rigid_actor_ = rigid_actor;

    set_linear_drag(rigid_body_component.linear_drag_);
    set_angular_drag(rigid_body_component.angular_drag_);

    set_kinematic(rigid_body_component.is_kinematic_);

    set_gravity_disabled(rigid_body_component.is_gravity_disabled_);

    rigid_actor->setSolverIterationCounts(settings.solver_iterations,
                                          settings.solver_velocity_iterations);

    rigid_actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
  }

  add_collider(entity);

  set_mass(rigid_body_component.mass_);

  rigid_actor_->userData = this;
  rigid_actor_->setName(entity.name().c_str());
}

PhysicActor::~PhysicActor()
{
  for (const auto &collider : colliders_)
  {
    collider->detach_from_actor(*rigid_actor_);
  }
  colliders_.clear();
  rigid_actor_->release();
  rigid_actor_ = nullptr;
}

Entity PhysicActor::entity() const { return entity_; }

void PhysicActor::sync_transform()
{
  auto       &transform_component = entity_.component<TransformComponent>();
  const auto &actor_pose          = rigid_actor_->getGlobalPose();
  transform_component.set_absolute_position(to_glm(actor_pose.p));
  transform_component.set_absolute_rotation(to_glm(actor_pose.q));
}

void PhysicActor::set_translation(const glm::vec3 &value, bool autowake)
{
  auto transform = rigid_actor_->getGlobalPose();
  transform.p    = to_physx(value);
  rigid_actor_->setGlobalPose(transform, autowake);

  if (rigid_body_type_ == RigidBodyType::Static)
  {
    sync_transform();
  }
}

glm::vec3 PhysicActor::get_translation() const
{
  return to_glm(rigid_actor_->getGlobalPose().p);
}

void PhysicActor::set_rotation(const glm::vec3 &value, bool autowake)
{
  auto transform = rigid_actor_->getGlobalPose();
  transform.q    = to_physx(glm::quat(value));
  rigid_actor_->setGlobalPose(transform, autowake);

  if (rigid_body_type_ == RigidBodyType::Static)
  {
    sync_transform();
  }
}

glm::vec3 PhysicActor::get_rotation() const
{
  return glm::eulerAngles(to_glm(rigid_actor_->getGlobalPose().q));
}

void PhysicActor::rotate(const glm::vec3 &value, bool autowake)
{
  physx::PxTransform transform = rigid_actor_->getGlobalPose();
  transform.q *= (physx::PxQuat{glm::radians(value.x), {1.0f, 0.0f, 0.0f}} *
                  physx::PxQuat{glm::radians(value.y), {0.0f, 1.0f, 0.0f}} *
                  physx::PxQuat{glm::radians(value.z), {0.0f, 0.0f, 1.0f}});
  rigid_actor_->setGlobalPose(transform, autowake);

  if (rigid_body_type_ == RigidBodyType::Static)
  {
    sync_transform();
  }
}

void PhysicActor::wake_up()
{
  if (is_dynamic())
  {
    rigid_actor_->is<physx::PxRigidDynamic>()->wakeUp();
  }
}

void PhysicActor::put_to_sleep()
{
  if (is_dynamic())
  {
    rigid_actor_->is<physx::PxRigidDynamic>()->putToSleep();
  }
}

bool PhysicActor::is_sleeping() const
{
  if (is_dynamic())
  {
    return rigid_actor_->is<physx::PxRigidDynamic>()->isSleeping();
  }
  return false;
}

void PhysicActor::set_mass(float mass)
{
  if (!is_dynamic())
  {
    return;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a rigid dynamic actor");
  entity_.component<RigidBodyComponent>().mass_ = mass;
  physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass);
}

float PhysicActor::get_mass() const
{
  if (!is_dynamic())
  {
    return 0.0f;
  }
  return rigid_actor_->is<physx::PxRigidDynamic>()->getMass();
}

float PhysicActor::get_inverse_mass() const
{
  if (!is_dynamic())
  {
    return 0.0f;
  }
  return rigid_actor_->is<physx::PxRigidDynamic>()->getInvMass();
}

glm::mat4 PhysicActor::get_center_of_mass() const
{
  if (!is_dynamic())
  {
    return glm::mat4{1.0f};
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  return to_glm(actor->getGlobalPose().transform(actor->getCMassLocalPose()));
}

glm::mat4 PhysicActor::get_local_center_of_mass() const
{
  if (!is_dynamic())
  {
    return glm::mat4{1.0f};
  }
  return to_glm(rigid_actor_->is<physx::PxRigidDynamic>()->getCMassLocalPose());
}

void PhysicActor::add_force(const glm::vec3 &force, ForceMode force_mode)
{
  DC_PROFILE_SCOPE("PhysicActor::add_force()");

  if (!is_dynamic() || is_kinematic())
  {
    DC_LOG_WARN(
        "Trying to add force to a non-dynamic or kinematic physic actor");
    return;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  actor->addForce(to_physx(force), ::to_physx(force_mode));
}

void PhysicActor::add_torque(const glm::vec3 &torque, ForceMode force_mode)
{
  DC_PROFILE_SCOPE("PhysicActor::add_torque()");

  if (!is_dynamic() || is_kinematic())
  {
    DC_LOG_WARN(
        "Trying to add torque to a non-dynamic or kinematic physic actor");
    return;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  actor->addTorque(to_physx(torque), ::to_physx(force_mode));
}

void PhysicActor::add_radial_impulse(const glm::vec3 &origin,
                                     float            radius,
                                     float            strength,
                                     FalloffMode      falloff_mode,
                                     bool             velocity_change)
{
  if (!is_dynamic() || is_kinematic())
  {
    return;
  }

  const auto      center_of_mass_transform = get_center_of_mass();
  const glm::vec3 center_of_mass           = center_of_mass_transform[3];
  auto            delta                    = center_of_mass - origin;

  const auto magnitude = glm::length(delta);
  if (magnitude > radius)
  {
    return;
  }

  delta = glm::normalize(delta);

  auto impulse_magnitude = strength;
  if (falloff_mode == FalloffMode::Linear)
  {
    impulse_magnitude *= (1.0f - (magnitude / radius));
  }

  const auto impulse = delta * impulse_magnitude;
  const auto mode =
      velocity_change ? ForceMode::VelocityChange : ForceMode::Impulse;
  add_force(impulse, mode);
}

void PhysicActor::set_linear_velocity(const glm::vec3 &value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to set velocity of non-dynamic physic actor");
    return;
  }

  rigid_actor_->is<physx::PxRigidDynamic>()->setLinearVelocity(to_physx(value));
}

glm::vec3 PhysicActor::get_linear_velocity() const
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to get velocity of non-dynamic physic actor.");
    return glm::vec3{0.0f};
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  return to_glm(actor->getLinearVelocity());
}

void PhysicActor::set_angular_velocity(const glm::vec3 &value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to set angular velocity of non-dynamic physic actor");
    return;
  }

  rigid_actor_->is<physx::PxRigidDynamic>()->setAngularVelocity(
      to_physx(value));
}

glm::vec3 PhysicActor::get_angular_velocity() const
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to get angular velocity of non-dynamic physic actor.");
    return glm::vec3{0.0f};
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  return to_glm(actor->getAngularVelocity());
}

void PhysicActor::set_max_linear_velocity(float value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN(
        "Trying to set max linear velocity of non-dynamic physic actor");
    return;
  }

  rigid_actor_->is<physx::PxRigidDynamic>()->setMaxLinearVelocity(value);
}

float PhysicActor::get_max_linear_velocity() const
{
  if (!is_dynamic())
  {
    DC_LOG_WARN(
        "Trying to get max linear velocity of non-dynamic physic actor.");
    return 0.0f;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  return actor->getMaxLinearVelocity();
}

void PhysicActor::set_max_angular_velocity(float value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN(
        "Trying to set max angular velocity of non-dynamic physic actor");
    return;
  }

  rigid_actor_->is<physx::PxRigidDynamic>()->setMaxAngularVelocity(value);
}

float PhysicActor::get_max_angular_velocity() const
{
  if (!is_dynamic())
  {
    DC_LOG_WARN(
        "Trying to get max angular velocity of non-dynamic physic actor.");
    return 0.0f;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  return actor->getMaxAngularVelocity();
}

void PhysicActor::set_linear_drag(float value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to set linear drag of non-dynamic physic actor");
    return;
  }

  entity_.component<RigidBodyComponent>().linear_drag_ = value;
  rigid_actor_->is<physx::PxRigidDynamic>()->setLinearDamping(value);
}

float PhysicActor::get_linear_drag() const
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to get linear drag of non-dynamic physic actor.");
    return 0.0f;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  return actor->getLinearDamping();
}

void PhysicActor::set_angular_drag(float value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to set angular drag of non-dynamic physic actor");
    return;
  }

  entity_.component<RigidBodyComponent>().angular_drag_ = value;
  rigid_actor_->is<physx::PxRigidDynamic>()->setAngularDamping(value);
}

float PhysicActor::get_angular_drag() const
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Trying to get angular drag of non-dynamic physic actor.");
    return 0.0f;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  return actor->getAngularDamping();
}

void PhysicActor::set_kinematic_target(const glm::vec3 &target_position,
                                       const glm::vec3 &target_rotation)
{
  if (!is_kinematic())
  {
    DC_LOG_WARN("Trying to set kinematic target for a non-kinematic actor.");
    return;
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  actor->setKinematicTarget(
      to_physx_transform(target_position, target_rotation));
}

glm::vec3 PhysicActor::get_kinematic_target_position() const
{
  if (!is_kinematic())
  {
    DC_LOG_WARN("Trying to get kinematic target for a non-kinematic actor.");
    return glm::vec3{0.0f, 0.0f, 0.0f};
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  physx::PxTransform target;
  DC_ASSERT(actor->getKinematicTarget(target), "Kinematic target not set");
  return to_glm(target.p);
}

glm::vec3 PhysicActor::get_kinematic_target_rotation() const
{
  if (!is_kinematic())
  {
    DC_LOG_WARN("Trying to get kinematic target for a non-kinematic actor.");
    return glm::vec3{0.0f, 0.0f, 0.0f};
  }

  const auto actor = rigid_actor_->is<physx::PxRigidDynamic>();
  DC_ASSERT(actor, "Not a dynamic actor");
  physx::PxTransform target;
  DC_ASSERT(actor->getKinematicTarget(target), "Kinematic target not set");
  return glm::eulerAngles(to_glm(target.q));
}

bool PhysicActor::is_dynamic() const
{
  return rigid_body_type_ == RigidBodyType::Dynamic;
}

void PhysicActor::set_kinematic(bool value)
{
  if (!is_dynamic())
  {
    DC_LOG_WARN("Static PhysicsActor can't be kinematic");
    return;
  }

  rigid_actor_->is<physx::PxRigidDynamic>()->setRigidBodyFlag(
      physx::PxRigidBodyFlag::eKINEMATIC,
      value);
  entity_.component<RigidBodyComponent>().is_kinematic_ = value;
}

bool PhysicActor::is_kinematic() const
{
  return is_dynamic() && entity_.component<RigidBodyComponent>().is_kinematic_;
}

void PhysicActor::set_gravity_disabled(bool value)
{
  rigid_actor_->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, value);
  entity_.component<RigidBodyComponent>().is_gravity_disabled_ = value;
}

bool PhysicActor::is_gravity_disabled() const
{
  return rigid_actor_->getActorFlags().isSet(
      physx::PxActorFlag::eDISABLE_GRAVITY);
}

void PhysicActor::add_collider(Entity entity, const glm::vec3 &offset)
{
  if (entity.has_component<BoxColliderComponent>())
  {
    add_box_collider(entity, offset);
  }
  if (entity.has_component<SphereColliderComponent>())
  {
    add_sphere_collider(entity, offset);
  }
  if (entity.has_component<CapsuleColliderComponent>())
  {
    add_capsule_collider(entity, offset);
  }
  if (entity.has_component<MeshColliderComponent>())
  {
    add_mesh_collider(entity, offset);
  }
}

void PhysicActor::add_box_collider(Entity entity, const glm::vec3 &offset)
{
  colliders_.push_back(
      std::make_shared<BoxCollider>(entity, rigid_actor_, offset));
}

void PhysicActor::add_sphere_collider(Entity entity, const glm::vec3 &offset)
{
  colliders_.push_back(
      std::make_shared<SphereCollider>(entity, rigid_actor_, offset));
}

void PhysicActor::add_capsule_collider(Entity entity, const glm::vec3 &offset)
{
  colliders_.push_back(
      std::make_shared<CapsuleCollider>(entity, rigid_actor_, offset));
}

void PhysicActor::add_mesh_collider(Entity entity,
                                    const glm::vec3 & /*offset*/)
{
  const auto is_convex = entity.component<MeshColliderComponent>().is_convex_;

  if (!is_convex && is_dynamic() && !is_kinematic())
  {
    DC_LOG_WARN("Can not add triangle mesh collider to dynamic non kinematic "
                "rigid actor");
    colliders_.push_back(
        std::make_shared<MeshCollider>(entity,
                                       rigid_actor_,
                                       MeshColliderType::Convex));
    return;
  }

  colliders_.push_back(std::make_shared<MeshCollider>(
      entity,
      rigid_actor_,
      (is_convex ? MeshColliderType::Convex : MeshColliderType::Triangle)));
}

physx::PxRigidActor *PhysicActor::px_rigid_actor() const
{
  return rigid_actor_;
}

} // namespace dc
