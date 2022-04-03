#pragma once

#include "frame_data.hpp"

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxPvd.h>

namespace dc
{

class PhysicEngine
{
public:
  PhysicEngine();
  ~PhysicEngine();

  void update(float delta_time);
  void render(SceneRenderInfo &scene_render_info);

  void set_debug_draw(bool value);
  bool debug_draw() const;

  void create_scene();
  void destroy_scene();

private:
  bool debug_draw_{false};

  physx::PxDefaultAllocator     px_allocator_;
  physx::PxDefaultErrorCallback px_error_callback_;

  physx::PxFoundation           *px_foundation_{};
  physx::PxPhysics              *px_physics_{};
  physx::PxDefaultCpuDispatcher *px_dispatcher_;
  physx::PxScene                *px_scene_{};
  physx::PxPvd                  *pvd_{};

  physx::PxControllerManager *px_controller_manager_{};

  void setup_debug_draw_for_scene();
};

} // namespace dc
