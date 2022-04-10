#pragma once

#include "entity.hpp"
#include "physic_actor.hpp"
#include "physic_collider.hpp"
#include "physic_material.hpp"

#include <PxActor.h>
#include <PxRigidActor.h>
#include <PxRigidBody.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace dc
{

enum class RigidBodyType
{
  Static,
  Dynamic,
};

enum class FalloffMode
{
  Constant,
  Linear,
};

enum class ForceMode : std::uint8_t
{
  Force = 0,
  Impulse,
  VelocityChange,
  Acceleration,
};

class RigidBody : public PhysicActor
{
public:
  RigidBody(Entity entity);
  ~RigidBody();

  Entity entity() const;

  void sync_transform();

  void      set_translation(const glm::vec3 &value, bool autowake = true);
  glm::vec3 get_translation() const;

  void      set_rotation(const glm::vec3 &value, bool autowake = true);
  glm::vec3 get_rotation() const;

  void rotate(const glm::vec3 &value, bool autowake = true);

  void wake_up();
  void put_to_sleep();
  bool is_sleeping() const;

  void  set_mass(float mass);
  float get_mass() const;

  float get_inverse_mass() const;

  glm::mat4 get_center_of_mass() const;
  glm::mat4 get_local_center_of_mass() const;

  void add_force(const glm::vec3 &force, ForceMode force_mode);
  void add_torque(const glm::vec3 &torque, ForceMode force_mode);
  void add_radial_impulse(const glm::vec3 &origin,
                          float            radius,
                          float            strength,
                          FalloffMode      falloff_mode = FalloffMode::Constant,
                          bool             velocity_change = false);

  void      set_linear_velocity(const glm::vec3 &value);
  glm::vec3 get_linear_velocity() const;

  void      set_angular_velocity(const glm::vec3 &value);
  glm::vec3 get_angular_velocity() const;

  void  set_max_linear_velocity(float value);
  float get_max_linear_velocity() const;

  void  set_max_angular_velocity(float);
  float get_max_angular_velocity() const;

  void  set_linear_drag(float value);
  float get_linear_drag() const;

  void  set_angular_drag(float value);
  float get_angular_drag() const;

  void set_kinematic_target(const glm::vec3 &target_position,
                            const glm::vec3 &target_rotation);

  glm::vec3 get_kinematic_target_position() const;
  glm::vec3 get_kinematic_target_rotation() const;

  bool is_dynamic() const;

  void set_kinematic(bool value);
  bool is_kinematic() const;

  void set_gravity_disabled(bool value);
  bool is_gravity_disabled() const;

  void add_collider(Entity entity, const glm::vec3 &offset = glm::vec3{0.0f});

  physx::PxRigidActor *px_rigid_actor() const;

private:
  Entity entity_;

  RigidBodyType rigid_body_type_;

  physx::PxRigidActor *rigid_actor_{};

  std::vector<std::shared_ptr<PhysicCollider>> colliders_;

  void add_box_collider(Entity entity, const glm::vec3 &offset);
  void add_sphere_collider(Entity entity, const glm::vec3 &offset);
  void add_capsule_collider(Entity entity, const glm::vec3 &offset);
  void add_mesh_collider(Entity entity, const glm::vec3 &offset);
};

} // namespace dc
