#include "physic_scene.hpp"
#include "assert.hpp"
#include "character_controller.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "frame_data.hpp"
#include "log.hpp"
#include "physic/character_controller_component.hpp"
#include "physic/rigid_body.hpp"
#include "physic_actor.hpp"
#include "physic_settings.hpp"
#include "physic_types.hpp"
#include "physx_helper.hpp"
#include "physx_sdk.hpp"
#include "profiling.hpp"
#include "rigid_body_component.hpp"

#include <PxRigidDynamic.h>
#include <PxSceneDesc.h>
#include <characterkinematic/PxController.h>
#include <extensions/PxBroadPhaseExt.h>
#include <extensions/PxDefaultSimulationFilterShader.h>

#include <functional>
#include <memory>

namespace
{

physx::PxBroadPhaseType::Enum to_px_broadphase(dc::BroadphaseType type)
{
  switch (type)
  {
  case dc::BroadphaseType::SweepAndPrune:
    return physx::PxBroadPhaseType::eSAP;
  case dc::BroadphaseType::MultiBoxPrune:
    return physx::PxBroadPhaseType::eMBP;
  case dc::BroadphaseType::AutomaticBoxPrune:
    return physx::PxBroadPhaseType::eABP;
  }

  return physx::PxBroadPhaseType::eABP;
}

physx::PxFrictionType::Enum to_px_friction_type(dc::FrictionType type)
{
  switch (type)
  {
  case dc::FrictionType::Patch:
    return physx::PxFrictionType::ePATCH;
  case dc::FrictionType::OneDirectional:
    return physx::PxFrictionType::eONE_DIRECTIONAL;
  case dc::FrictionType::TwoDirectional:
    return physx::PxFrictionType::eTWO_DIRECTIONAL;
  }

  return physx::PxFrictionType::ePATCH;
}

} // namespace

namespace dc
{

PhysicScene::PhysicScene()
    : sub_step_size_{default_physic_settings().fixed_timestep}
{
  is_debug_draw_ =
      Engine::instance()->config()->config_value_bool("Physic", "debug", false);

  const auto &settings = default_physic_settings();

  const auto sdk = PhysXSdk::get_instance();

  physx::PxSceneDesc scene_desc{sdk->get_physics()->getTolerancesScale()};

  scene_desc.flags |=
      physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
  scene_desc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
  scene_desc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

  scene_desc.gravity        = to_physx(settings.gravity);
  scene_desc.filterShader   = physx::PxDefaultSimulationFilterShader;
  scene_desc.broadPhaseType = to_px_broadphase(settings.broadphase_algorithm);
  scene_desc.frictionType   = to_px_friction_type(settings.friction_model);
  scene_desc.cpuDispatcher  = sdk->get_cpu_dispatcher();
  scene_desc.simulationEventCallback = &contact_listener_;

  DC_ASSERT(scene_desc.isValid(), "Scene desc invalid");

  scene_              = sdk->get_physics()->createScene(scene_desc);
  controller_manager_ = PxCreateControllerManager(*scene_);

  DC_ASSERT(scene_, "Could not create PhysX scene");

  create_regions();
  set_debug_draw(is_debug_draw_);
}

PhysicScene::~PhysicScene()
{
  clear();
  px_release(controller_manager_);
  px_release(scene_);
}

void PhysicScene::update(float delta_time)
{
  DC_PROFILE_SCOPE("physicscene::update()");

  const auto is_simulation_advanced = step_simulation(delta_time);
  if (!is_simulation_advanced)
  {
    return;
  }

  process_active_actors();
}

RigidBody *PhysicScene::create_rigid_body(Entity entity)
{
  DC_PROFILE_SCOPE("PhysicScene::create_actor()");

  DC_ASSERT(scene_, "Scene is not set");

  const auto existing_actor = get_rigid_body(entity);
  if (existing_actor)
  {
    return existing_actor;
  }

  DC_ASSERT(entity.has_component<RigidBodyComponent>(),
            "Entity has no rigid body component");
  auto &rigid_body_component = entity.component<RigidBodyComponent>();

  auto       actor     = std::make_unique<RigidBody>(entity);
  const auto actor_ptr = actor.get();

  rigid_body_component.physic_actor_ = actor_ptr;

  scene_->addActor(*actor->px_rigid_actor());
  actors_[entity.id()] = std::move(actor);

  return actor_ptr;
}

RigidBody *PhysicScene::get_rigid_body(Entity entity)
{
  // TODO: Maybe could just use actor from component

  DC_PROFILE_SCOPE("PhysicScene::get_actor()");
  const auto iter = actors_.find(entity.id());
  if (iter != actors_.end())
  {
    return iter->second.get();
  }
  return nullptr;
}

void PhysicScene::remove_rigid_body(Entity entity)
{
  DC_PROFILE_SCOPE("PhysicScene::remove_actor()");

  remove_actor_from_scene(entity);
  actors_.erase(entity.id());
}

CharacterController *PhysicScene::create_controller(Entity entity)
{
  DC_PROFILE_SCOPE("PhysicScene::create_controller()");

  DC_ASSERT(scene_, "Scene is not set");

  const auto existing_controller = get_controller(entity);
  if (existing_controller)
  {
    return existing_controller;
  }

  auto controller =
      std::make_unique<CharacterController>(entity, *controller_manager_);
  const auto controller_ptr = controller.get();

  controllers_[entity.id()] = std::move(controller);

  return controller_ptr;
}

CharacterController *PhysicScene::get_controller(Entity entity)
{
  // TODO: Maybe could just use actor from component

  DC_PROFILE_SCOPE("PhysicScene::get_controller()");

  const auto iter = controllers_.find(entity.id());
  if (iter != controllers_.end())
  {
    return iter->second.get();
  }
  return nullptr;
}

void PhysicScene::remove_controller(Entity entity)
{
  DC_PROFILE_SCOPE("PhysicScene::remove_controller()");
  remove_controller_from_scene(entity);
  controllers_.erase(entity.id());
}

void PhysicScene::create_regions()
{
  const auto &settings = default_physic_settings();
  if (settings.broadphase_algorithm == BroadphaseType::AutomaticBoxPrune)
  {
    return;
  }

  const auto regionBounds =
      new physx::PxBounds3[settings.world_bounds_subdivisions *
                           settings.world_bounds_subdivisions];
  physx::PxBounds3 globalBounds(to_physx(settings.world_bounds_min),
                                to_physx(settings.world_bounds_max));

  const auto regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(
      regionBounds,
      globalBounds,
      settings.world_bounds_subdivisions);

  for (unsigned i = 0; i < regionCount; i++)
  {
    physx::PxBroadPhaseRegion region;
    region.bounds = regionBounds[i];
    scene_->addBroadPhaseRegion(region);
  }
}

void PhysicScene::clear()
{
  DC_ASSERT(scene_, "Can not clear without a scene");

  // Remove controllers
  for (const auto &controller : controllers_)
  {
    remove_controller_from_scene(controller.second->get_entity());
  }
  controllers_.clear();

  // Remove actors
  for (const auto &actor : actors_)
  {
    remove_actor_from_scene(actor.second->get_entity());
  }
  actors_.clear();
}

bool PhysicScene::step_simulation(float delta_time)
{
  calc_substeps(delta_time);

  for (unsigned i = 0; i < num_sub_steps_; ++i)
  {
    scene_->simulate(sub_step_size_);
    scene_->fetchResults(true);
  }

  return num_sub_steps_ != 0;
}

void PhysicScene::calc_substeps(float delte_time)
{
  if (accumulator_ > sub_step_size_)
  {
    accumulator_ = 0.0f;
  }

  accumulator_ += delte_time;
  if (accumulator_ < sub_step_size_)
  {
    num_sub_steps_ = 0;
    return;
  }

  num_sub_steps_ =
      glm::min(static_cast<std::uint32_t>(accumulator_ / sub_step_size_),
               max_sub_steps);
  accumulator_ -= static_cast<float>(num_sub_steps_ * sub_step_size_);
}
void PhysicScene::process_active_actors()
{
  unsigned   active_actors_count{};
  const auto active_actors = scene_->getActiveActors(active_actors_count);
  for (unsigned i = 0; i < active_actors_count; ++i)
  {
    const auto actor = static_cast<PhysicActor *>(active_actors[i]->userData);
    DC_ASSERT(actor, "Actor can't be nullptr");
    if (actor->physic_actor_type() == PhysicActorType::RigidBody)
    {
      const auto rigid_body = static_cast<RigidBody *>(actor);
      if (!rigid_body->is_sleeping())
      {
        rigid_body->sync_transform();
      }
    }
    else if (actor->physic_actor_type() == PhysicActorType::CharacterController)
    {
      const auto character_controller =
          static_cast<CharacterController *>(actor);
      character_controller->sync_transform();
    }
    else
    {
      DC_FAIL("Unknown physic actor type");
    }
  }
}

void PhysicScene::set_debug_draw(bool value)
{
  is_debug_draw_ = value;

  const auto scale_value      = value ? 1.0f : 0.0f;
  const auto actor_axes_value = value ? 2.0f : 0.0f;

  scene_->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE,
                                    scale_value);
  scene_->setVisualizationParameter(
      physx::PxVisualizationParameter::eACTOR_AXES,
      actor_axes_value);
  scene_->setVisualizationParameter(
      physx::PxVisualizationParameter::eCOLLISION_SHAPES,
      scale_value);
}

bool PhysicScene::is_debug_draw() const { return is_debug_draw_; }

std::vector<DebugLineInfo> PhysicScene::get_debug_lines() const
{
  if (!is_debug_draw_)
  {
    return {};
  }

  const auto color_to_vec3 = [](physx::PxU32 color) -> glm::vec3
  {
    const auto r = ((color >> 16) & 0xff) / 0xff;
    const auto g = ((color >> 8) & 0xff) / 0xff;
    const auto b = ((color >> 0) & 0xff) / 0xff;

    return {r, g, b};
  };
  const auto &render_buffer = scene_->getRenderBuffer();
  const auto &debug_lines   = render_buffer.getLines();

  std::vector<DebugLineInfo> debug_line_infos(render_buffer.getNbLines());
  for (unsigned i = 0; i < render_buffer.getNbLines(); ++i)
  {
    const auto &line = debug_lines[i];

    auto &debug_line_info        = debug_line_infos[i];
    debug_line_info.start_       = to_glm(line.pos0);
    debug_line_info.end_         = to_glm(line.pos1);
    debug_line_info.start_color_ = color_to_vec3(line.color0);
    debug_line_info.end_color_   = color_to_vec3(line.color1);
  }

  return debug_line_infos;
}

void PhysicScene::remove_actor_from_scene(Entity entity)
{
  DC_PROFILE_SCOPE("PhysicScene::remove_actor_from_scene()");

  const auto actor = get_rigid_body(entity);
  if (!actor)
  {
    return;
  }

  auto &rigid_body_component         = entity.component<RigidBodyComponent>();
  rigid_body_component.physic_actor_ = nullptr;

  scene_->removeActor(*actor->px_rigid_actor());
}

void PhysicScene::remove_controller_from_scene(Entity entity)
{
  DC_PROFILE_SCOPE("PhysicScene::remove_controller_from_scene()");

  const auto controller = get_controller(entity);
  if (!controller)
  {
    return;
  }

  auto &component       = entity.component<CharacterControllerComponent>();
  component.controller_ = nullptr;
}

} // namespace dc
