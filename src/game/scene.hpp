#pragma once

#include "event.hpp"
#include "system.hpp"
#include "texture_cache.hpp"

#include <entt/entt.hpp>

#include <memory>

class Entity;
class Scene;

/**
 * Event gets fired after the scene has been loaded
 */
class SceneLoadedEvent : public Event
{
public:
  static EventId id;

  std::shared_ptr<Scene> scene_;

  SceneLoadedEvent(std::shared_ptr<Scene> scene);
};

/**
 * Event gets fired after the scene got unloaded
 */
class SceneUnloadedEvent : public Event
{
public:
  static EventId id;

  std::shared_ptr<Scene> scene_;

  SceneUnloadedEvent(std::shared_ptr<Scene> scene);
};

class Scene : public std::enable_shared_from_this<Scene>
{
public:
  static std::shared_ptr<Scene> create();

  static std::shared_ptr<Scene>
  load_from_file(const std::filesystem::path &file_path,
                 TextureCache                &texture_cache);

  void init();
  void update(float delta_time);
  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info);

  bool on_event(const Event &event);

  template <typename TSystem, typename... TArgs>
  void create_system(TArgs &&...args)
  {
    systems_.emplace_back(
        std::make_unique<TSystem>(std::forward<TArgs>(args)...));
  }

  Entity create_entity(const std::string &name);

  entt::registry &registry();

private:
  entt::registry                       registry_;
  std::vector<std::unique_ptr<System>> systems_;

  Scene() = default;

  void init_systems();

  void load_scene(const aiScene *ai_scene, TextureCache &texture_cache);
  void load_node(const aiScene        *ai_scene,
                 aiNode               *ai_node,
                 std::optional<Entity> parent_entity,
                 TextureCache         &texture_cache);
  void load_mesh(const aiScene *ai_scene,
                 Entity         parent_entity,
                 aiMesh        *ai_mesh,
                 TextureCache  &texture_cache);
};
