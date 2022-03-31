#include "entity_panel.hpp"
#include "camera_component.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "env_map_asset.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui.hpp"
#include "imgui_panel.hpp"
#include "mesh_component.hpp"
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
      if (ImGui::Selectable("Mesh"))
      {
        entity_.add_component<MeshComponent>();
      }
      if (ImGui::Selectable("Skinned mesh"))
      {
        entity_.add_component<SkinnedMeshComponent>();
      }
      if (ImGui::Selectable("Camera"))
      {
        entity_.add_component<CameraComponent>();
      }
      if (ImGui::Selectable("Directional light"))
      {
        entity_.add_component<DirectionalLightComponent>();
      }
      if (ImGui::Selectable("Point light"))
      {
        entity_.add_component<PointLightComponent>();
      }
      if (ImGui::Selectable("Sky"))
      {
        entity_.add_component<SkyComponent>();
      }
      if (ImGui::Selectable("Script"))
      {
        entity_.add_component<ScriptComponent>();
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
    if (component.skinned_mesh_)
    {
      skinned_mesh_name = component.skinned_mesh_->asset().id();
    }
    if (imgui_input("Skinned mesh asset",
                    skinned_mesh_name,
                    ImGuiInputTextFlags_EnterReturnsTrue))
    {
      auto handle = Engine::instance()->asset_cache()->load_asset(
          Asset{skinned_mesh_name});
      component.skinned_mesh_ =
          std::dynamic_pointer_cast<SkinnedMeshAssetHandle>(handle);
    }

    if (component.skinned_mesh_)
    {
      const auto skinned_mesh = component.skinned_mesh_->get();
      auto       is_endless   = skinned_mesh->is_animation_endless();
      if (imgui_input("Endless", is_endless))
      {
        skinned_mesh->set_animation_endless(is_endless);
      }

      auto       animation_name = skinned_mesh->current_animation_name();
      if (imgui_input("Animation",
                      animation_name,
                      ImGuiInputTextFlags_EnterReturnsTrue))
      {
        if (animation_name.empty())
        {
          skinned_mesh->stop_current_animation();
        }
        else
        {
          skinned_mesh->play_animation(animation_name);
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
      ScriptComponentConstructEvent event{entity_};
      Engine::instance()->event_manager()->fire_event(event);
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
