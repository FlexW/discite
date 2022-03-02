#pragma once

#include "entt/entity/fwd.hpp"
#include "event.hpp"
#include "serialization.hpp"
#include "system.hpp"
#include "texture_cache.hpp"
#include "uuid.hpp"

#include <entt/entt.hpp>

#include <filesystem>
#include <memory>
#include <unordered_map>

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

  // void load_from_file(const std::filesystem::path &file_path,
  //                     TextureCache                &texture_cache);

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
  Entity create_entity(const std::string &name, Uuid uuid);

  template <typename... T> decltype(auto) all_entities_with()
  {
    return registry_.view<T...>();
  }

  Entity get_or_create_entity(Uuid uuid);
  Entity entity(Uuid uuid);
  bool   exists(Uuid uuid) const;

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description);
  AssetDescription read(const std::filesystem::path &file_path);

private:
  // TODO: Consider creating a proper API for entities
  friend Entity;

  std::unordered_map<Uuid, entt::entity> uuid_to_entity_map_;
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
