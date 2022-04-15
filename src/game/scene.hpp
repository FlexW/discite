#pragma once

#include "component_types.hpp"
#include "entt/entity/fwd.hpp"
#include "event.hpp"
#include "serialization.hpp"
#include "system.hpp"
#include "uuid.hpp"

#include <entt/entt.hpp>

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace dc
{

class Entity;

class Scene : public std::enable_shared_from_this<Scene>
{
public:
  static std::shared_ptr<Scene> create();

  Entity create_entity(const std::string &name);
  Entity create_entity(const std::string &name, Uuid uuid);

  void remove_entity(Uuid uuid);

  template <typename... T> decltype(auto) all_entities_with()
  {
    return registry_.view<T...>();
  }

  Entity get_or_create_entity(Uuid uuid);
  Entity entity(Uuid uuid);
  bool   exists(Uuid uuid) const;

  void remove_entities();

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description);
  AssetDescription read(const std::filesystem::path &file_path);

private:
  // TODO: Consider creating a proper API for entities
  friend Entity;

  std::vector<Uuid> entities_to_remove;

  std::unordered_map<Uuid, entt::entity> uuid_to_entity_map_;
  entt::registry                         registry_;

  Scene();

  void init_systems();

  void fire_component_construct_event(entt::entity  entity,
                                      ComponentType component_type);
  void fire_component_destroy_event(entt::entity  entity,
                                    ComponentType component_type);

  void on_script_component_construct(entt::registry &registry,
                                     entt::entity    entity);
  void on_script_component_destroy(entt::registry &registry,
                                   entt::entity    entity);

  void on_rigid_body_component_construct(entt::registry &registry,
                                         entt::entity    entity);
  void on_rigid_body_component_destroy(entt::registry &registry,
                                       entt::entity    entity);

  void on_character_controller_component_construct(entt::registry &registry,
                                                   entt::entity    entity);
  void on_character_controller_component_destroy(entt::registry &registry,
                                                 entt::entity    entity);

  void on_box_collider_component_construct(entt::registry &registry,
                                           entt::entity    entity);
  void on_box_collider_component_destroy(entt::registry &registry,
                                         entt::entity    entity);

  void on_sphere_collider_component_construct(entt::registry &registry,
                                              entt::entity    entity);
  void on_sphere_collider_component_destroy(entt::registry &registry,
                                            entt::entity    entity);

  void on_capsule_collider_component_construct(entt::registry &registry,
                                               entt::entity    entity);
  void on_capsule_collider_component_destroy(entt::registry &registry,
                                             entt::entity    entity);

  void on_mesh_collider_component_construct(entt::registry &registry,
                                            entt::entity    entity);
  void on_mesh_collider_component_destroy(entt::registry &registry,
                                          entt::entity    entity);

  void on_audio_source_component_construct(entt::registry &registry,
                                           entt::entity    entity);
  void on_audio_source_component_destroy(entt::registry &registry,
                                         entt::entity    entity);
};

} // namespace dc
