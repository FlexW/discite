#include "physic_system.hpp"
#include "component_types.hpp"
#include "log.hpp"
#include "physic/physic_actor.hpp"
#include "physic/physic_controller.hpp"
#include "physic/physic_scene.hpp"
#include "physic/rigid_body_component.hpp"
#include "scene.hpp"
#include "scene_events.hpp"

#include <entt/entity/fwd.hpp>
#include <memory>

namespace dc
{

void PhysicSystem::init() {}

void PhysicSystem::update(float delta_time)
{
  const auto scene = scene_.lock();
  if (!physic_scene_ || !scene)
  {
    return;
  }

  physic_scene_->update(delta_time);
}

void PhysicSystem::render(SceneRenderInfo &scene_render_info,
                          ViewRenderInfo & /*view_render_info*/)
{
  if (physic_scene_ && physic_scene_->is_debug_draw())
  {
    scene_render_info.add_debug_lines(physic_scene_->get_debug_lines());
  }
}

bool PhysicSystem::on_event(const Event &event)
{
  const auto event_id = event.id();
  if (event_id == SceneLoadedEvent::id)
  {
    on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }
  else if (event_id == ComponentConstructEvent::id)
  {
    on_component_construct(
        dynamic_cast<const ComponentConstructEvent &>(event));
  }
  else if (event_id == ComponentDestroyEvent::id)
  {
    on_component_destroy(dynamic_cast<const ComponentDestroyEvent &>(event));
  }

  return false;
}

void PhysicSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;

  // Create new physic scene
  physic_scene_ = std::make_unique<PhysicScene>();
  create_physic_actors();
}

void PhysicSystem::on_scene_unloaded(const SceneUnloadedEvent & /*event*/)
{
  physic_scene_ = nullptr;
}

void PhysicSystem::on_component_construct(const ComponentConstructEvent &event)
{
  if (event.component_type_ == ComponentType::RigidBody)
  {
    physic_scene_->create_actor(event.entity_);
  }
  else if (event.component_type_ == ComponentType::BoxCollider ||
           event.component_type_ == ComponentType::SphereCollider ||
           event.component_type_ == ComponentType::CapsuleCollider ||
           event.component_type_ == ComponentType::MeshCollider)
  {
    if (!event.entity_.has_component<RigidBodyComponent>())
    {
      DC_LOG_WARN("Trying to add collider to component without rigid body");
      return;
    }
    const auto &rigid_body_component =
        event.entity_.component<RigidBodyComponent>();
    const auto actor = rigid_body_component.physic_actor_;
    DC_ASSERT(actor, "No physic actor");
    actor->add_collider(event.entity_);
  }
}

void PhysicSystem::on_component_destroy(const ComponentDestroyEvent &event)
{
  if (event.component_type_ == ComponentType::RigidBody)
  {
    physic_scene_->remove_actor(event.entity_);
  }
}

void PhysicSystem::create_physic_actors()
{
  DC_ASSERT(physic_scene_, "Physic scene not loaded");

  auto scene = scene_.lock();
  if (scene)
  {
    auto view = scene->all_entities_with<RigidBodyComponent>();
    for (const auto &entity : view)
    {
      physic_scene_->create_actor(Entity{entity, scene});
    }
  }
}

} // namespace dc
