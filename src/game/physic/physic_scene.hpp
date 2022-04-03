#pragma once

#include "entity.hpp"
#include "math.hpp"
#include "physic_actor.hpp"
#include "physic_controller.hpp"
#include "physic_types.hpp"
#include "physx_contact_listener.hpp"
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

  PhysicActor *create_actor(Entity entity);
  PhysicActor *get_actor(Entity entity);
  void         remove_actor(Entity entity);

  PhysicController *create_controller(Entity entity);
  PhysicController *get_controller(Entity entity);
  void              remove_controller(Entity entity);

  void process_active_actors();
  void process_controllers();

  void set_debug_draw(bool value);
  bool is_debug_draw() const;

  std::vector<DebugLineInfo> get_debug_lines() const;

private:
  bool is_debug_draw_{true};

  std::unordered_map<Uuid, std::unique_ptr<PhysicActor>> actors_;
  std::unordered_map<Uuid, std::unique_ptr<PhysicActor>> controllers_;

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
