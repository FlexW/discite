#include "entity_panel.hpp"
#include "camera_component.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "env_map_asset.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui.hpp"
#include "imgui_panel.hpp"
#include "model_component.hpp"
#include "point_light.hpp"
#include "point_light_component.hpp"
#include "scene.hpp"
#include "scene_panel.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"

#include <memory>

namespace dc
{

EntityPanel::EntityPanel() : ImGuiPanel{"Entity"} {}

void EntityPanel::on_render()
{
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
      if (ImGui::Selectable("Model"))
      {
        entity_.add_component<ModelComponent>();
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
      ImGui::EndPopup();
    }
  }

  // render other components information
  if (entity_.has_component<ModelComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Model");

    auto       &component = entity_.component<ModelComponent>();
    std::string mesh_name;
    if (component.model_)
    {
      mesh_name = component.model_->asset().id();
    }
    if (imgui_input("Mesh", mesh_name, ImGuiInputTextFlags_EnterReturnsTrue))
    {
      auto handle =
          Engine::instance()->asset_cache()->load_asset(Asset{mesh_name});
      component.model_ = std::dynamic_pointer_cast<MeshAssetHandle>(handle);
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
  }

  if (entity_.has_component<PointLightComponent>())
  {
    ImGui::Separator();
    ImGui::Text("Point light");
    auto &point_light_component = entity_.component<PointLightComponent>();
    imgui_input("Color", point_light_component.color_);
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
