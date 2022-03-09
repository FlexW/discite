#include "scene.hpp"
#include "camera_component.hpp"
#include "directional_light_component.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "guid_component.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "model_component.hpp"
#include "name_component.hpp"
#include "point_light_component.hpp"
#include "relationship_component.hpp"
#include "render_system.hpp"
#include "serialization.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"
#include "uuid.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>

namespace dc
{

EventId SceneLoadedEvent::id = 0x87a8e8c9;

SceneLoadedEvent::SceneLoadedEvent(std::shared_ptr<Scene> scene)
    : Event{id},
      scene_{scene}
{
}

EventId SceneUnloadedEvent::id = 0xa26517c8;

SceneUnloadedEvent::SceneUnloadedEvent(std::shared_ptr<Scene> scene)
    : Event{id},
      scene_{scene}
{
}

std::shared_ptr<Scene> Scene::create()
{
  return std::shared_ptr<Scene>(new Scene);
}

void Scene::init_systems()
{
  systems_.emplace_back(std::make_unique<RenderSystem>(shared_from_this()));
}

void Scene::init()
{
  init_systems();

  for (const auto &system : systems_)
  {
    system->init();
  }
}

void Scene::update(float delta_time)
{
  for (const auto &system : systems_)
  {
    system->update(delta_time);
  }
}

void Scene::render(SceneRenderInfo &scene_render_info,
                   ViewRenderInfo  &view_render_info)
{
  for (const auto &system : systems_)
  {
    system->render(scene_render_info, view_render_info);
  }
}

bool Scene::on_event(const Event &event)
{
  for (const auto &system : systems_)
  {
    system->on_event(event);
  }

  return false;
}

Entity Scene::create_entity(const std::string &name)
{
  return create_entity(name, 0);
}

Entity Scene::create_entity(const std::string &name, Uuid uuid)
{
  Entity     entity{registry_.create(), shared_from_this()};
  const auto entity_name = name.empty() ? "Entity" : name;
  entity.add_component<GuidComponent>(uuid);
  entity.add_component<NameComponent>(name);
  entity.add_component<TransformComponent>();
  entity.add_component<RelationshipComponent>();

  uuid_to_entity_map_[entity.component<GuidComponent>().id_] =
      entity.entity_handle();

  return entity;
}

Entity Scene::get_or_create_entity(Uuid uuid)
{
  if (uuid == 0)
  {
    return {};
  }

  if (exists(uuid))
  {
    return entity(uuid);
  }

  return create_entity("", uuid);
}

Entity Scene::entity(Uuid uuid)
{
  assert(uuid != 0);
  const auto iter = uuid_to_entity_map_.find(uuid);
  assert(iter != uuid_to_entity_map_.end());
  return Entity{iter->second, shared_from_this()};
}

bool Scene::exists(Uuid uuid) const
{
  if (uuid == 0)
  {
    return false;
  }

  const auto iter = uuid_to_entity_map_.find(uuid);
  return iter != uuid_to_entity_map_.end();
}

void Scene::save(const std::filesystem::path &file_path,
                 const AssetDescription      &asset_description)
{
  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);

  const auto &view = registry_.view<GuidComponent>();
  write_value(file, static_cast<std::uint64_t>(view.size()));
  for (const auto &e : view)
  {
    Entity entity{e, shared_from_this()};
    entity.component<GuidComponent>().save(file);
    entity.component<NameComponent>().save(file);
    entity.component<TransformComponent>().save(file);
    entity.component<RelationshipComponent>().save(file);

    if (entity.has_component<ModelComponent>())
    {
      write_string(file, "*model*");
      entity.component<ModelComponent>().save(file);
    }

    if (entity.has_component<SkyComponent>())
    {
      write_string(file, "*sky*");
      entity.component<SkyComponent>().save(file);
    }

    if (entity.has_component<PointLightComponent>())
    {
      write_string(file, "*pointlight*");
      entity.component<PointLightComponent>().save(file);
    }

    if (entity.has_component<DirectionalLightComponent>())
    {
      write_string(file, "*directionallight*");
      entity.component<DirectionalLightComponent>().save(file);
    }

    if (entity.has_component<CameraComponent>())
    {
      write_string(file, "*camera*");
      entity.component<CameraComponent>().save(file);
    }

    write_string(file, "*end*");
    }
}

AssetDescription Scene::read(const std::filesystem::path &file_path)
{
  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  AssetDescription asset_description{};
  asset_description.read(file);

  std::uint64_t entity_count{};
  read_value(file, entity_count);
  for (std::uint64_t i{0}; i < entity_count; ++i)
  {
    GuidComponent uuid_component;
    uuid_component.read(file);

    auto entity = get_or_create_entity(uuid_component.id_);
    entity.set_id(uuid_component.id_);

    auto &name_component = entity.component<NameComponent>();
    name_component.read(file);

    auto &transform_component = entity.component<TransformComponent>();
    transform_component.read(file);

    auto &relationship_component = entity.component<RelationshipComponent>();
    relationship_component.read(file, *this);

    std::string marker;
    read_string(file, marker);

    while (marker != "*end*")
    {
      if (marker == "*model*")
      {
        auto &component = entity.add_component<ModelComponent>();
        component.read(file);
      }
      else if (marker == "*pointlight*")
      {
        auto &component = entity.add_component<PointLightComponent>();
        component.read(file);
      }
      else if (marker == "*directionallight*")
      {
        auto &component = entity.add_component<DirectionalLightComponent>();
        component.read(file);
      }
      else if (marker == "*camera*")
      {
        auto &component = entity.add_component<CameraComponent>();
        component.read(file);
      }
      else if (marker == "*sky*")
      {
        auto &component = entity.add_component<SkyComponent>();
        component.read(file);
      }
      read_string(file, marker);
    }
  }

  return asset_description;
}

} // namespace dc
