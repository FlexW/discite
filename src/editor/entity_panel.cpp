#include "entity_panel.hpp"
#include "camera_component.hpp"
#include "component_types.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "env_map_asset.hpp"
#include "imgui.hpp"
#include "imgui_panel.hpp"
#include "mesh_component.hpp"
#include "physic/box_collider.hpp"
#include "physic/box_collider_component.hpp"
#include "physic/capsule_collider.hpp"
#include "physic/capsule_collider_component.hpp"
#include "physic/character_controller_component.hpp"
#include "physic/mesh_collider_component.hpp"
#include "physic/physic_actor.hpp"
#include "physic/physic_collider.hpp"
#include "physic/physic_material.hpp"
#include "physic/physic_scene.hpp"
#include "physic/rigid_body_component.hpp"
#include "physic/sphere_collider.hpp"
#include "physic/sphere_collider_component.hpp"
#include "point_light.hpp"
#include "point_light_component.hpp"
#include "profiling.hpp"
#include "scene.hpp"
#include "scene_events.hpp"
#include "scene_panel.hpp"
#include "script/script_component.hpp"
#include "skinned_mesh_asset.hpp"
#include "skinned_mesh_component.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"

#include <memory>

namespace
{

void draw_physic_material_properties(dc::PhysicCollider &collider)
{
  const auto         px_physic_material = collider.get_physic_material();
  dc::PhysicMaterial physic_material{};
  physic_material.static_friction_  = px_physic_material->getStaticFriction();
  physic_material.dynamic_friction_ = px_physic_material->getDynamicFriction();
  physic_material.bounciness_       = px_physic_material->getRestitution();

  if (dc::imgui_input("Static friction", physic_material.static_friction_))
  {
    collider.set_physic_material(physic_material);
  }
  if (dc::imgui_input("Dynamic friction", physic_material.dynamic_friction_))
  {
    collider.set_physic_material(physic_material);
  }
  if (dc::imgui_input("Bounciness", physic_material.bounciness_))
  {
    collider.set_physic_material(physic_material);
  }
}

} // namespace

namespace dc
{

EntityPanel::EntityPanel() : ImGuiPanel{"Entity"} {}

void EntityPanel::on_render()
{
  DC_PROFILE_SCOPE("EntityPanel::on_render()");

  if (!entity_.valid())
  {
    return;
  }

  {
    // id
    const auto id = entity_.id();
    ImGui::Text("%lu", id);
  }

  {
    // Remove
    ImGui::SameLine();
    if (ImGui::Button("Remove entity"))
    {
      entity_.remove();
    }
  }

  {
    // name
    auto name = entity_.name();
    if (imgui_input("Name", name))
    {
      entity_.set_name(name);
    }
  }

  ImGui::Separator();

  {
    // transformation info
    auto position = entity_.position();
    if (imgui_input("Position", position))
    {
      entity_.set_position(position);
    }

    const auto rotation = entity_.rotation();
    glm::vec3  rotation_degrees{glm::degrees(rotation.x),
                               glm::degrees(rotation.y),
                               glm::degrees(rotation.z)};
    if (imgui_input("Rotation", rotation_degrees))
    {
      entity_.set_rotation(glm::vec3{glm::radians(rotation_degrees.x),
                                     glm::radians(rotation_degrees.y),
                                     glm::radians(rotation_degrees.z)});
    }

    auto scale = entity_.scale();
    if (imgui_input("Scale", scale))
    {
      entity_.set_scale(scale);
    }
  }

  {
    // add components
    ImGui::Separator();

    if (ImGui::Button("Add component"))
    {
      ImGui::OpenPopup("select_component_popup");
    }

    if (ImGui::BeginPopup("select_component_popup"))
    {
      if (!entity_.has_component<MeshComponent>())
      {
        if (ImGui::Selectable("Mesh"))
        {
          entity_.add_component<MeshComponent>();
        }
      }
      if (!entity_.has_component<SkinnedMeshComponent>())
      {
        if (ImGui::Selectable("Skinned mesh"))
        {
          entity_.add_component<SkinnedMeshComponent>();
        }
      }
      if (!entity_.has_component<CameraComponent>())
      {
        if (ImGui::Selectable("Camera"))
        {
          entity_.add_component<CameraComponent>();
        }
      }
      if (!entity_.has_component<DirectionalLightComponent>())
      {
        if (ImGui::Selectable("Directional light"))
        {
          entity_.add_component<DirectionalLightComponent>();
        }
      }
      if (!entity_.has_component<PointLightComponent>())
      {
        if (ImGui::Selectable("Point light"))
        {
          entity_.add_component<PointLightComponent>();
        }
      }
      if (!entity_.has_component<SkyComponent>())
      {
        if (ImGui::Selectable("Sky"))
        {
          entity_.add_component<SkyComponent>();
        }
      }
      if (!entity_.has_component<ScriptComponent>())
      {
        if (ImGui::Selectable("Script"))
        {
          entity_.add_component<ScriptComponent>();
        }
      }
      if (!entity_.has_component<CharacterControllerComponent>())
      {
        if (ImGui::Selectable("Character controller"))
        {
          entity_.add_component<CharacterControllerComponent>();
        }
      }
      if (!entity_.has_component<RigidBodyComponent>())
      {
        if (ImGui::Selectable("Rigid body (static)"))
        {
          entity_.add_component<RigidBodyComponent>(RigidBodyType::Static);
        }
        else if (ImGui::Selectable("Rigid body (dynamic)"))
        {
          entity_.add_component<RigidBodyComponent>(RigidBodyType::Dynamic);
        }
      }
      if (!entity_.has_component<BoxColliderComponent>())
      {
        if (ImGui::Selectable("Box collider"))
        {
          entity_.add_component<BoxColliderComponent>();
        }
      }
      if (!entity_.has_component<SphereColliderComponent>())
      {
        if (ImGui::Selectable("Sphere collider"))
        {
          entity_.add_component<SphereColliderComponent>();
        }
      }
      if (!entity_.has_component<CapsuleColliderComponent>())
      {
        if (ImGui::Selectable("Capsule collider"))
        {
          entity_.add_component<CapsuleColliderComponent>();
        }
      }
      if (!entity_.has_component<MeshColliderComponent>())
      {
        if (ImGui::Selectable("Mesh collider"))
        {
          entity_.add_component<MeshColliderComponent>();
        }
      }
      ImGui::EndPopup();
    }
  }

  // render other components information
  if (entity_.has_component<MeshComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Mesh");

    auto       &component = entity_.component<MeshComponent>();
    std::string mesh_name;
    if (component.model_)
    {
      mesh_name = component.model_->asset().id();
    }
    if (imgui_input("Mesh asset",
                    mesh_name,
                    ImGuiInputTextFlags_EnterReturnsTrue))
    {
      auto handle =
          Engine::instance()->asset_cache()->load_asset(Asset{mesh_name});
      component.model_ = std::dynamic_pointer_cast<MeshAssetHandle>(handle);
    }
  }

  if (entity_.has_component<SkinnedMeshComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Skinned mesh");

    auto &component = entity_.component<SkinnedMeshComponent>();

    std::string skinned_mesh_name;
    if (component.skinned_mesh_asset())
    {
      skinned_mesh_name = component.skinned_mesh_asset()->asset().id();
    }
    if (imgui_input("Skinned mesh asset",
                    skinned_mesh_name,
                    ImGuiInputTextFlags_EnterReturnsTrue))
    {
      auto handle = Engine::instance()->asset_cache()->load_asset(
          Asset{skinned_mesh_name});
      component.set_skinned_mesh_asset(
          std::dynamic_pointer_cast<SkinnedMeshAssetHandle>(handle));
    }

    if (component.animation_state())
    {
      const auto animation_state = component.animation_state();
      auto       is_endless      = animation_state->is_animation_endless();
      if (imgui_input("Endless", is_endless))
      {
        animation_state->set_animation_endless(is_endless);
      }

      auto animation_name = animation_state->current_animation_name();
      if (imgui_input("Animation",
                      animation_name,
                      ImGuiInputTextFlags_EnterReturnsTrue))
      {
        if (animation_name.empty())
        {
          animation_state->stop_current_animation();
        }
        else
        {
          animation_state->play_animation(animation_name);
        }
      }
    }
  }

  if (entity_.has_component<CameraComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Camera");
  }

  if (entity_.has_component<DirectionalLightComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Directional light");
    auto &directional_light_component =
        entity_.component<DirectionalLightComponent>();
    imgui_input("Color", directional_light_component.color_);
    imgui_input("Multiplier", directional_light_component.multiplier_);
  }

  if (entity_.has_component<PointLightComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Point light");
    auto &point_light_component = entity_.component<PointLightComponent>();
    imgui_input("Color", point_light_component.color_);
    imgui_input("Multiplier", point_light_component.multiplier_);
    imgui_input("Radius", point_light_component.radius_);
    imgui_input("Falloff", point_light_component.falloff_);
    auto cast_shadow = point_light_component.cast_shadow();
    if (imgui_input("Cast shadow", cast_shadow))
    {
      point_light_component.set_cast_shadow(cast_shadow);
    }
  }

  if (entity_.has_component<SkyComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Sky");
    auto &component = entity_.component<SkyComponent>();

    std::string environment_name;
    if (component.environment_)
    {
      environment_name = component.environment_->asset().id();
    }
    if (imgui_input("Environment",
                    environment_name,
                    ImGuiInputTextFlags_EnterReturnsTrue))
    {
      auto env_map_handle = Engine::instance()->asset_cache()->load_asset(
          Asset{environment_name});
      component.environment_ =
          std::dynamic_pointer_cast<EnvMapAssetHandle>(env_map_handle);
    }
  }

  if (entity_.has_component<ScriptComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Script");

    auto &component = entity_.component<ScriptComponent>();
    if (imgui_input("Module",
                    component.module_name_,
                    ImGuiInputTextFlags_EnterReturnsTrue))
    {
      ComponentConstructEvent event{entity_, ComponentType::Script};
      Engine::instance()->event_manager()->fire_event(event);
    }
  }

  if (entity_.has_component<CharacterControllerComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Character controller");

    auto &component = entity_.component<CharacterControllerComponent>();

    const auto controller = component.controller_;
    if (controller)
    {
      auto slope_limit = component.slope_limit_degree_;
      if (imgui_input("Slope limit", slope_limit))
      {
        controller->set_slope_limit(slope_limit);
      }

      auto step_offset = component.step_offset_;
      if (imgui_input("Step offset", step_offset))
      {
        controller->set_step_offset(step_offset);
      }

      auto disable_gravity = component.disable_gravity_;
      if (imgui_input("Disable gravity", disable_gravity))
      {
        controller->set_has_gravity(!disable_gravity);
      }
    }
  }

  if (entity_.has_component<RigidBodyComponent>())
  {
    ImGui::Separator();
    auto &component = entity_.component<RigidBodyComponent>();

    const auto is_dynamic = component.body_type_ == RigidBodyType::Dynamic;
    const auto text =
        is_dynamic ? "Rigid body (dynamic)" : "Rigid body (static)";
    ImGui::Text(text);
    const auto actor = component.physic_actor_;
    if (component.physic_actor_ && is_dynamic)
    {
      auto mass = actor->get_mass();
      if (imgui_input("Mass", mass))
      {
        actor->set_mass(mass);
      }
      auto linear_drag = actor->get_linear_drag();
      if (imgui_input("Linear drag", linear_drag))
      {
        actor->set_linear_drag(linear_drag);
      }
      auto angular_drag = actor->get_angular_drag();
      if (imgui_input("Angular drag", angular_drag))
      {
        actor->set_angular_drag(angular_drag);
      }
      auto is_kinematic = actor->is_kinematic();
      if (imgui_input("Kinematic", is_kinematic))
      {
        actor->set_kinematic(is_kinematic);
      }
      auto is_gravity_disabled = actor->is_gravity_disabled();
      if (imgui_input("Gravity disabled", is_gravity_disabled))
      {
        actor->set_gravity_disabled(is_gravity_disabled);
      }
    }
  }

  if (entity_.has_component<BoxColliderComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Box collider");
    auto      &component = entity_.component<BoxColliderComponent>();
    const auto box_collider         = component.box_collider_;
    const auto character_controller = component.character_controller_;
    if (box_collider)
    {
      auto size = box_collider->get_size();
      if (imgui_input("Size", size))
      {
        box_collider->set_size(size);
      }
      auto offset = box_collider->get_offset();
      if (imgui_input("Offset", offset))
      {
        box_collider->set_offset(offset);
      }
      auto is_trigger = box_collider->is_trigger();
      if (imgui_input("Trigger", is_trigger))
      {
        box_collider->set_trigger(is_trigger);
      }

      draw_physic_material_properties(*box_collider);
    }
    else if (character_controller)
    {
      auto size = component.size_;
      if (imgui_input("Size", size))
      {
        character_controller->set_size(size);
      }

      auto offset = component.offset_;
      if (imgui_input("Offset", offset))
      {
        character_controller->set_offset(offset);
      }
    }
  }

  if (entity_.has_component<SphereColliderComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Sphere collider");
    auto      &component = entity_.component<SphereColliderComponent>();
    const auto collider  = component.sphere_collider_;
    if (collider)
    {
      auto radius = collider->get_radius();
      if (imgui_input("Radius", radius))
      {
        collider->set_radius(radius);
      }
      auto offset = collider->get_offset();
      if (imgui_input("Offset", offset))
      {
        collider->set_offset(offset);
      }
      auto is_trigger = collider->is_trigger();
      if (imgui_input("Trigger", is_trigger))
      {
        collider->set_trigger(is_trigger);
      }

      draw_physic_material_properties(*collider);
    }
  }

  if (entity_.has_component<CapsuleColliderComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Capsule collider");
    auto      &component = entity_.component<CapsuleColliderComponent>();
    const auto collider  = component.capsule_collider_;
    const auto character_controller = component.character_controller_;
    if (collider)
    {
      auto radius = collider->get_radius();
      if (imgui_input("Radius", radius))
      {
        collider->set_radius(radius);
      }
      auto height = collider->get_height();
      if (imgui_input("Height", height))
      {
        collider->set_height(height);
      }
      auto offset = collider->get_offset();
      if (imgui_input("Offset", offset))
      {
        collider->set_offset(offset);
      }
      auto is_trigger = collider->is_trigger();
      if (imgui_input("Trigger", is_trigger))
      {
        collider->set_trigger(is_trigger);
      }

      draw_physic_material_properties(*collider);
    }
    else if (character_controller)
    {
      auto radius = component.radius_;
      if (imgui_input("Radius", radius))
      {
        character_controller->set_radius(radius);
      }

      auto height = component.height_;
      if (imgui_input("Height", height))
      {
        character_controller->set_height(height);
      }

      auto offset = component.offset_;
      if (imgui_input("Offset", offset))
      {
        character_controller->set_offset(offset);
      }
    }
  }

  if (entity_.has_component<MeshColliderComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Mesh collider");
    auto      &component = entity_.component<MeshColliderComponent>();
    const auto collider  = component.mesh_collider_;
    if (collider)
    {
      auto is_trigger = collider->is_trigger();
      if (imgui_input("Trigger", is_trigger))
      {
        collider->set_trigger(is_trigger);
      }
      // if (imgui_input("Convex", component.is_convex_))
      // {
      //   DC_LOG_WARN("Changing mesh collider convex state is not supported
      //   yet");
      // }
      draw_physic_material_properties(*collider);
    }
  }
}

bool EntityPanel::on_event(const Event &event)
{
  const auto event_id = event.id();

  if (event_id == EntitySelectedEvent::id)
  {
    return on_entity_selected(dynamic_cast<const EntitySelectedEvent &>(event));
  }
  else if (event_id == SceneUnloadedEvent::id)
  {
    return on_scene_unloaded(dynamic_cast<const SceneUnloadedEvent &>(event));
  }

  return false;
}

bool EntityPanel::on_entity_selected(const EntitySelectedEvent &event)
{
  entity_ = event.entity_;
  return false;
}

bool EntityPanel::on_scene_unloaded(const SceneUnloadedEvent & /*event*/)
{
  entity_ = {};
  return false;
}

} // namespace dc
