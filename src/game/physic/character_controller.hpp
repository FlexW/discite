#pragma once

#include "entity.hpp"
#include "physic_actor.hpp"
#include "physic_material.hpp"
#include "physic_settings.hpp"

#include <PxMaterial.h>
#include <characterkinematic/PxController.h>
#include <characterkinematic/PxControllerManager.h>

namespace dc
{

class CharacterController : public PhysicActor
{
public:
  CharacterController(Entity                      entity,
                      physx::PxControllerManager &controller_manager);
  ~CharacterController();

  void sync_transform();

  void set_has_gravity(bool value);
  void set_slope_limit(float slope_limit_degree);
  void set_step_offset(float offset);

  void      set_position(const glm::vec3 &value);
  glm::vec3 get_position() const;

  void set_offset(const glm::vec3 &value);

  float     get_speed_down() const;
  bool      is_grounded() const;

  void move(const glm::vec3 &displacement);
  void jump(float jump_power);

  void update(float delta_time);

  void set_size(const glm::vec3 &size);

  void set_radius(float radius);
  void set_height(float height);
  void resize(float height);

  physx::PxController *px_controller() const;

private:
  physx::PxMaterial *material_{};

  physx::PxController              *controller_{};
  physx::PxControllerCollisionFlags collision_flags_{};

  float     speed_down_{0.0f};
  glm::vec3 displacement_{0.0f};
  float     gravity_{glm::length(default_physic_settings().gravity)};
  bool      has_gravity_{true};

  void create_capsule_collider_controller(
      physx::PxControllerManager &controller_manager);
};

} // namespace dc
