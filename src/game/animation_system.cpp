#include "animation_system.hpp"
#include "engine.hpp"
#include "game_layer.hpp"
#include "log.hpp"
#include "profiling.hpp"
#include "skinned_mesh_component.hpp"

namespace dc
{

void AnimationSystem::init()
{
  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  if (game_layer)
  {
    const auto scene = game_layer->scene();
    if (scene)
    {
      scene_ = scene->get();
    }
  }
}

void AnimationSystem::update(float delta_time)
{
  DC_PROFILE_SCOPE("AnimationSystem::update()");

  const auto scene = scene_.lock();
  if (!scene)
  {
    DC_LOG_WARN("Scene is not valid. Will not animate meshes");
  }

  auto view = scene->all_entities_with<SkinnedMeshComponent>();
  for (const auto entity : view)
  {
    auto &skinned_mesh_component = view.get<SkinnedMeshComponent>(entity);

    const auto &animation_state = skinned_mesh_component.animation_state();
    if (!animation_state)
    {
      continue;
    }

    animation_state->compute_bone_transforms(delta_time);
  }
}

void AnimationSystem::render(SceneRenderInfo & /*scene_render_info*/,
                             ViewRenderInfo & /*view_render_info*/)
{
}

bool AnimationSystem::on_event(const Event &event)
{
  const auto event_id = event.id();
  if (event_id == SceneLoadedEvent::id)
  {
    on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }

  return false;
}

void AnimationSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
}

} // namespace dc
