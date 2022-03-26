#pragma once

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
  entt::registry                         registry_;

  Scene();

  void init_systems();

  void on_construct_script_component(entt::registry &registry,
                                     entt::entity    entity);
  void on_destroy_script_component(entt::registry &registry,
                                   entt::entity    entity);
};

} // namespace dc
