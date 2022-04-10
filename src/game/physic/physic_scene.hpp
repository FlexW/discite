#pragma once

#include "character_controller.hpp"
#include "entity.hpp"
#include "math.hpp"
#include "physic_actor.hpp"
#include "physic_types.hpp"
#include "physx_contact_listener.hpp"
#include "rigid_body.hpp"
#include "uuid.hpp"

#include <PxScene.h>
#include <characterkinematic/PxControllerManager.h>

#include <cstdint>
#include <functional>
#include <memory>

namespace dc
{

class PhysicScene
{
public:
  PhysicScene();
  ~PhysicScene();

  void update(float delta_time);

  RigidBody *create_rigid_body(Entity entity);
  RigidBody *get_rigid_body(Entity entity);
  void       remove_rigid_body(Entity entity);

  CharacterController *create_controller(Entity entity);
  CharacterController *get_controller(Entity entity);
  void                 remove_controller(Entity entity);

  void process_active_actors();

  void set_debug_draw(bool value);
  bool is_debug_draw() const;

  std::vector<DebugLineInfo> get_debug_lines() const;

private:
  bool is_debug_draw_{true};

  std::unordered_map<Uuid, std::unique_ptr<RigidBody>>           actors_;
  std::unordered_map<Uuid, std::unique_ptr<CharacterController>> controllers_;

  physx::PxScene             *scene_{};
  physx::PxControllerManager *controller_manager_{};
  PhysXContactListener        contact_listener_;

  float          sub_step_size_;
  float          accumulator_{0.0f};
  uint32_t       num_sub_steps_{0};
  const uint32_t max_sub_steps{8};

  void clear();
  void create_regions();

  bool step_simulation(float delta_time);
  void calc_substeps(float delte_time);

  void remove_actor_from_scene(Entity entity);
  void remove_controller_from_scene(Entity entity);
};

} // namespace dc
