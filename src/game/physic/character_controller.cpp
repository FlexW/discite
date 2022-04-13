#include "character_controller.hpp"
#include "assert.hpp"
#include "box_collider_component.hpp"
#include "capsule_collider_component.hpp"
#include "character_controller_component.hpp"
#include "entity.hpp"
#include "log.hpp"
#include "physic_actor.hpp"
#include "physic_settings.hpp"
#include "physx_helper.hpp"
#include "physx_sdk.hpp"
#include "transform_component.hpp"

#include <PxRigidDynamic.h>
#include <characterkinematic/PxBoxController.h>
#include <characterkinematic/PxCapsuleController.h>
#include <characterkinematic/PxController.h>
#include <characterkinematic/PxControllerManager.h>

namespace dc
{

CharacterController::CharacterController(
    Entity                      entity,
    physx::PxControllerManager &controller_manager)
    : PhysicActor{entity, PhysicActorType::CharacterController}
{
  DC_ASSERT(entity.has_component<CharacterControllerComponent>(),
            "Entity has not character controller component");

  auto &controller_component = entity.component<CharacterControllerComponent>();
  controller_component.controller_ = this;

  if (entity.has_component<CapsuleColliderComponent>())
  {
    create_capsule_collider_controller(controller_manager);
  }
  else if (entity.has_component<BoxColliderComponent>())
  {
    auto &collider = entity.component<BoxColliderComponent>();
    DC_ASSERT(!collider.character_controller_ && !collider.box_collider_,
              "Collider is already used");
    collider.character_controller_ = this;
    material_      = PhysXSdk::get_instance()->get_physics()->createMaterial(
        collider.physic_material_.static_friction_,
        collider.physic_material_.dynamic_friction_,
        collider.physic_material_.bounciness_);

    physx::PxBoxControllerDesc desc{};
    desc.position          = to_physx_ext(entity.position() + collider.offset_);
    desc.halfHeight        = (collider.size_.y * entity.scale().y) / 2.0f;
    desc.halfSideExtent    = (collider.size_.x * entity.scale().x) / 2.0f;
    desc.halfForwardExtent = (collider.size_.z * entity.scale().z) / 2.0f;
    desc.nonWalkableMode =
        physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
    desc.slopeLimit = std::max(
        0.0f,
        glm::cos(glm::radians(controller_component.slope_limit_degree_)));
    desc.stepOffset    = controller_component.step_offset_;
    desc.contactOffset = 0.01f;
    desc.material      = material_;
    desc.upDirection   = {0.0f, 1.0f, 0.0f};
    DC_ASSERT(desc.isValid(), "Box controller desc invalid");

    controller_ = controller_manager.createController(desc);
    DC_ASSERT(controller_, "Controller could not be created");
  }
  else
  {
    DC_LOG_DEBUG("No collider component set for character controller. Add "
                 "capsule collider.");
    entity.add_component<CapsuleColliderComponent>();
    create_capsule_collider_controller(controller_manager);
  }

  has_gravity_ = !controller_component.disable_gravity_;

  controller_->getActor()->userData = this;
}

CharacterController::~CharacterController()
{
  if (controller_)
  {
    px_release(controller_);
  }

  if (material_)
  {
    px_release(material_);
  }
}

void CharacterController::sync_transform()
{
  auto &component = get_entity().component<TransformComponent>();

  component.set_absolute_position(get_position());
}

void CharacterController::set_has_gravity(bool value)
{
  has_gravity_ = value;
  get_entity().component<CharacterControllerComponent>().disable_gravity_ =
      !has_gravity_;
}

void CharacterController::set_slope_limit(float slope_limit_degree)
{
  if (controller_)
  {
    controller_->setSlopeLimit(
        std::max(0.0f, glm::cos(glm::radians(slope_limit_degree))));
  }
  get_entity().component<CharacterControllerComponent>().slope_limit_degree_ =
      slope_limit_degree;
}

void CharacterController::set_step_offset(float offset)
{
  if (controller_)
  {
    controller_->setStepOffset(offset);
  }
  get_entity().component<CharacterControllerComponent>().step_offset_ = offset;
}

void CharacterController::set_position(const glm::vec3 &value)
{
  DC_ASSERT(controller_, "No controller set");
  if (controller_->getType() == physx::PxControllerShapeType::eBOX)
  {
    DC_ASSERT(get_entity().has_component<BoxColliderComponent>(),
              "No box collider component set");

    auto &collider = get_entity().component<BoxColliderComponent>();
    controller_->setPosition(to_physx_ext(value + collider.offset_));
  }
  else if (controller_->getType() == physx::PxControllerShapeType::eCAPSULE)
  {
    DC_ASSERT(get_entity().has_component<CapsuleColliderComponent>(),
              "No capsule collider component set");

    auto &collider = get_entity().component<CapsuleColliderComponent>();
    controller_->setPosition(to_physx_ext(value + collider.offset_));
  }
  else
  {
    DC_FAIL("Unknown shape type");
  }

  get_entity().component<TransformComponent>().set_absolute_position(value);
}

void CharacterController::set_offset(const glm::vec3 &value)
{
  if (controller_->getType() == physx::PxControllerShapeType::eBOX)
  {
    get_entity().component<BoxColliderComponent>().offset_ = value;
    set_position(get_entity().position());
  }
  else if (controller_->getType() == physx::PxControllerShapeType::eCAPSULE)
  {
    get_entity().component<CapsuleColliderComponent>().offset_ = value;
    set_position(get_entity().position());
  }
  else
  {
    DC_FAIL("Unknown shape type");
  }
}

glm::vec3 CharacterController::get_position() const
{
  DC_ASSERT(controller_, "No controller set");

  auto position = to_glm(controller_->getPosition());
  if (get_entity().has_component<CapsuleColliderComponent>())
  {
    const auto &component = get_entity().component<CapsuleColliderComponent>();
    position -= component.offset_;
  }
  else if (get_entity().has_component<BoxColliderComponent>())
  {
    const auto &component = get_entity().component<BoxColliderComponent>();
    position -= component.offset_;
  }

  return position;
}

float CharacterController::get_speed_down() const { return speed_down_; }

bool CharacterController::is_grounded() const
{
  return collision_flags_ & physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
}

void CharacterController::move(const glm::vec3 &displacement)
{
  displacement_ += displacement;
}

void CharacterController::jump(float jump_power)
{
  speed_down_ = -1.0f * jump_power;
}

void CharacterController::update(float delta_time)
{
  DC_ASSERT(controller_, "No controller set");

  if (has_gravity_)
  {
    speed_down_ += gravity_ * delta_time;
  }

  const auto displacement =
      displacement_ -
      to_glm(controller_->getUpDirection()) * speed_down_ * delta_time;

  physx::PxControllerFilters filters{};
  collision_flags_ = controller_->move(to_physx(displacement),
                                       0.0f,
                                       static_cast<physx::PxF32>(delta_time),
                                       filters);

  if (is_grounded())
  {
    // setting speed back to zero here would be more technically correct,
    // but a small non-zero gives better results (e.g. lessens jerkyness when
    // walking down a slope)

    speed_down_ = gravity_ * 0.01f;
  }

  displacement_ = glm::vec3{0.0f};
}

physx::PxController *CharacterController::px_controller() const
{
  return controller_;
}

void CharacterController::create_capsule_collider_controller(
    physx::PxControllerManager &controller_manager)
{
  auto        entity = get_entity();
  const auto &controller_component =
      entity.component<CharacterControllerComponent>();
  auto &collider = entity.component<CapsuleColliderComponent>();
  DC_ASSERT(!collider.character_controller_ && !collider.capsule_collider_,
            "Collider is already used");
  collider.character_controller_ = this;
  material_ = PhysXSdk::get_instance()->get_physics()->createMaterial(
      collider.physic_material_.static_friction_,
      collider.physic_material_.dynamic_friction_,
      collider.physic_material_.bounciness_);

  const auto radius_scale = glm::max(entity.scale().x, entity.scale().z);

  physx::PxCapsuleControllerDesc desc{};
  desc.position        = to_physx_ext(entity.position() + collider.offset_);
  desc.height          = collider.height_ * entity.scale().y;
  desc.radius          = collider.radius_ * radius_scale;
  desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
  desc.climbingMode    = physx::PxCapsuleClimbingMode::eCONSTRAINED;
  desc.slopeLimit      = std::max(
      0.0f,
      glm::cos(glm::radians(controller_component.slope_limit_degree_)));
  desc.stepOffset    = controller_component.step_offset_;
  desc.contactOffset = 0.01f;
  desc.material      = material_;
  desc.upDirection   = {0.0f, 1.0f, 0.0f};
  DC_ASSERT(desc.isValid(), "Capsule controller desc invalid");

  controller_ = controller_manager.createController(desc);
  DC_ASSERT(controller_, "Controller could not be created");
}

void CharacterController::set_size(const glm::vec3 &size)
{
  auto entity = get_entity();
  DC_ASSERT(controller_, "No controller set");
  DC_ASSERT(controller_->getType() == physx::PxControllerShapeType::eBOX,
            "Not a box controller");
  DC_ASSERT(entity.has_component<BoxColliderComponent>(), "Not a box collider");
  auto &collider = entity.component<BoxColliderComponent>();

  const auto box_controller =
      static_cast<physx::PxBoxController *>(controller_);

  box_controller->setHalfHeight((size.y * entity.scale().y) / 2.0f);
  box_controller->setHalfSideExtent((size.x * entity.scale().x) / 2.0f);
  box_controller->setHalfForwardExtent((size.z * entity.scale().z) / 2.0f);

  collider.size_ = size;
}

void CharacterController::set_radius(float radius)
{
  auto entity = get_entity();

  DC_ASSERT(controller_, "No controller set");
  DC_ASSERT(controller_->getType() == physx::PxControllerShapeType::eCAPSULE,
            "Not a capsule controller");
  DC_ASSERT(entity.has_component<CapsuleColliderComponent>(),
            "Not a capsule collider");

  auto &collider = entity.component<CapsuleColliderComponent>();

  const auto capsule_controller =
      static_cast<physx::PxCapsuleController *>(controller_);

  const auto radius_scale = glm::max(entity.scale().x, entity.scale().z);

  capsule_controller->setRadius(radius * radius_scale);

  collider.radius_ = radius;
}

void CharacterController::set_height(float height)
{
  auto entity = get_entity();

  DC_ASSERT(controller_, "No controller set");
  DC_ASSERT(controller_->getType() == physx::PxControllerShapeType::eCAPSULE,
            "Not a capsule controller");
  DC_ASSERT(entity.has_component<CapsuleColliderComponent>(),
            "Not a capsule collider");

  auto &collider = entity.component<CapsuleColliderComponent>();

  const auto capsule_controller =
      static_cast<physx::PxCapsuleController *>(controller_);

  capsule_controller->setHeight(height * entity.scale().y);

  collider.height_ = height;
}

void CharacterController::resize(float height)
{
  auto entity = get_entity();

  DC_ASSERT(controller_, "No controller set");
  DC_ASSERT(controller_->getType() == physx::PxControllerShapeType::eCAPSULE,
            "Not a capsule controller");
  DC_ASSERT(entity.has_component<CapsuleColliderComponent>(),
            "Not a capsule collider");

  auto &collider = entity.component<CapsuleColliderComponent>();

  const auto capsule_controller =
      static_cast<physx::PxCapsuleController *>(controller_);

  capsule_controller->resize(height * entity.scale().y);

  collider.height_ = height;
}

} // namespace dc
