#include "scene.hpp"
#include "camera_component.hpp"
#include "component_types.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "guid_component.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "mesh_component.hpp"
#include "name_component.hpp"
#include "physic/box_collider_component.hpp"
#include "physic/capsule_collider_component.hpp"
#include "physic/mesh_collider_component.hpp"
#include "physic/rigid_body_component.hpp"
#include "physic/sphere_collider_component.hpp"
#include "point_light_component.hpp"
#include "profiling.hpp"
#include "relationship_component.hpp"
#include "render_system.hpp"
#include "scene_events.hpp"
#include "script/script_component.hpp"
#include "serialization.hpp"
#include "skinned_mesh_component.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"
#include "uuid.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>

namespace dc
{

std::shared_ptr<Scene> Scene::create()
{
  return std::shared_ptr<Scene>(new Scene);
}

Scene::Scene()
{
  registry_.on_construct<ScriptComponent>()
      .connect<&Scene::on_script_component_construct>(this);
  registry_.on_destroy<ScriptComponent>()
      .connect<&Scene::on_script_component_destroy>(this);

  registry_.on_construct<RigidBodyComponent>()
      .connect<&Scene::on_rigid_body_component_construct>(this);
  registry_.on_destroy<RigidBodyComponent>()
      .connect<&Scene::on_rigid_body_component_destroy>(this);

  registry_.on_construct<BoxColliderComponent>()
      .connect<&Scene::on_box_collider_component_construct>(this);
  registry_.on_destroy<BoxColliderComponent>()
      .connect<&Scene::on_box_collider_component_destroy>(this);

  registry_.on_construct<SphereColliderComponent>()
      .connect<&Scene::on_sphere_collider_component_construct>(this);
  registry_.on_destroy<SphereColliderComponent>()
      .connect<&Scene::on_sphere_collider_component_destroy>(this);

  registry_.on_construct<CapsuleColliderComponent>()
      .connect<&Scene::on_capsule_collider_component_construct>(this);
  registry_.on_destroy<CapsuleColliderComponent>()
      .connect<&Scene::on_capsule_collider_component_destroy>(this);

  registry_.on_construct<MeshColliderComponent>()
      .connect<&Scene::on_mesh_collider_component_construct>(this);
  registry_.on_destroy<MeshColliderComponent>()
      .connect<&Scene::on_mesh_collider_component_destroy>(this);
}

Entity Scene::create_entity(const std::string &name)
{
  return create_entity(name, 0);
}

Entity Scene::create_entity(const std::string &name, Uuid uuid)
{
  DC_PROFILE_SCOPE("Scene::create_entity()");

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

void Scene::remove_entity(Uuid uuid)
{
  DC_PROFILE_SCOPE("Scene::remove_entity()");
  const auto &e = entity(uuid);
  registry_.destroy(e.entity_handle());
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
  DC_PROFILE_SCOPE("Scene::entity()");

  DC_ASSERT(uuid != 0, "Invalid uuid");
  const auto iter = uuid_to_entity_map_.find(uuid);
  DC_ASSERT(iter != uuid_to_entity_map_.end(), "Invalid entity");
  return Entity{iter->second, shared_from_this()};
}

bool Scene::exists(Uuid uuid) const
{
  DC_PROFILE_SCOPE("Scene::exists()");

  if (uuid == 0)
  {
    return false;
  }

  const auto iter = uuid_to_entity_map_.find(uuid);
  return iter != uuid_to_entity_map_.end();
}

void Scene::save(const std::filesystem::path &file_path,
                 const AssetDescription &     asset_description)
{
  DC_PROFILE_SCOPE("Scene::save()");

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

    if (entity.has_component<MeshComponent>())
    {
      write_string(file, "*model*");
      entity.component<MeshComponent>().save(file);
    }

    if (entity.has_component<SkinnedMeshComponent>())
    {
      write_string(file, "*skinnedmesh*");
      entity.component<SkinnedMeshComponent>().save(file);
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

    if (entity.has_component<ScriptComponent>())
    {
      write_string(file, "*script*");
      entity.component<ScriptComponent>().save(file);
    }

    write_string(file, "*end*");
  }
}

AssetDescription Scene::read(const std::filesystem::path &file_path)
{
  DC_PROFILE_SCOPE("Scene::read()");

  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  if (std::feof(file))
  {
    throw std::runtime_error{"File is empty: " + file_path.string()};
  }

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
        MeshComponent component{};
        component.read(file);
        entity.add_component<MeshComponent>(std::move(component));
      }
      else if (marker == "*skinnedmesh*")
      {
        SkinnedMeshComponent component{};
        component.read(file);
        entity.add_component<SkinnedMeshComponent>(component);
      }
      else if (marker == "*pointlight*")
      {
        PointLightComponent component{};
        component.read(file);
        entity.add_component<PointLightComponent>(std::move(component));
      }
      else if (marker == "*directionallight*")
      {
        DirectionalLightComponent component{};
        component.read(file);
        entity.add_component<DirectionalLightComponent>(std::move(component));
      }
      else if (marker == "*camera*")
      {
        CameraComponent component{};
        component.read(file);
        entity.add_component<CameraComponent>(std::move(component));
      }
      else if (marker == "*sky*")
      {
        SkyComponent component{};
        component.read(file);
        entity.add_component<SkyComponent>(std::move(component));
      }
      else if (marker == "*script*")
      {
        ScriptComponent component{};
        component.read(file);
        entity.add_component<ScriptComponent>(std::move(component));
      }
      read_string(file, marker);
    }
  }

  return asset_description;
}

void Scene::fire_component_destroy_event(entt::entity      entity,
                                         dc::ComponentType component_type)
{
  dc::ComponentDestroyEvent event{dc::Entity{entity, shared_from_this()},
                                  component_type};
  dc::Engine::instance()->event_manager()->fire_event(event);
}

void Scene::fire_component_construct_event(entt::entity      entity,
                                           dc::ComponentType component_type)
{
  dc::ComponentConstructEvent event{dc::Entity{entity, shared_from_this()},
                                    component_type};
  dc::Engine::instance()->event_manager()->fire_event(event);
}

void Scene::on_script_component_construct(entt::registry & /*registry*/,
                                          entt::entity entity)
{
  fire_component_construct_event(entity, ComponentType::Script);
}

void Scene::on_script_component_destroy(entt::registry & /*registry*/,
                                        entt::entity entity)
{
  fire_component_destroy_event(entity, ComponentType::Script);
}

void Scene::on_rigid_body_component_construct(entt::registry & /*registry*/,
                                              entt::entity entity)
{
  fire_component_construct_event(entity, ComponentType::RigidBody);
}

void Scene::on_rigid_body_component_destroy(entt::registry & /*registry*/,
                                            entt::entity entity)
{
  fire_component_destroy_event(entity, ComponentType::RigidBody);
}

void Scene::on_box_collider_component_construct(entt::registry & /*registry*/,
                                                entt::entity entity)
{
  fire_component_construct_event(entity, ComponentType::BoxCollider);
}

void Scene::on_box_collider_component_destroy(entt::registry & /*registry*/,
                                              entt::entity entity)
{
  fire_component_destroy_event(entity, ComponentType::BoxCollider);
}

void Scene::on_sphere_collider_component_construct(
    entt::registry & /*registry*/,
    entt::entity entity)
{
  fire_component_construct_event(entity, ComponentType::SphereCollider);
}

void Scene::on_sphere_collider_component_destroy(entt::registry & /*registry*/,
                                                 entt::entity entity)
{
  fire_component_destroy_event(entity, ComponentType::SphereCollider);
}

void Scene::on_capsule_collider_component_construct(
    entt::registry & /*registry*/,
    entt::entity entity)
{
  fire_component_construct_event(entity, ComponentType::CapsuleCollider);
}

void Scene::on_capsule_collider_component_destroy(entt::registry & /*registry*/,
                                                  entt::entity entity)
{
  fire_component_destroy_event(entity, ComponentType::CapsuleCollider);
}

void Scene::on_mesh_collider_component_construct(entt::registry & /*registry*/,
                                                 entt::entity entity)
{
  fire_component_construct_event(entity, ComponentType::MeshCollider);
}

void Scene::on_mesh_collider_component_destroy(entt::registry & /*registry*/,
                                               entt::entity entity)
{
  fire_component_destroy_event(entity, ComponentType::MeshCollider);
}

} // namespace dc
