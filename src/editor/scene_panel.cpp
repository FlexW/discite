#include "scene_panel.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "imgui.h"
#include "imgui_panel.hpp"
#include "name_component.hpp"
#include "scene.hpp"

#include <limits>
#include <memory>
#include <string>

EventId EntitySelectedEvent::id = 0x5832c140;

EntitySelectedEvent::EntitySelectedEvent(Entity entity)
    : Event{id},
      entity_{entity}
{
}

ScenePanel::ScenePanel() : ImGuiPanel{"Scene"} {}

void ScenePanel::on_render()
{
  if (ImGui::Button("Add entity"))
  {
    const auto scene = scene_.lock();
    if (scene)
    {
      scene->create_entity("New entity");
    }
  }

  const auto float_min = std::numeric_limits<float>::min();
  if (ImGui::BeginListBox("Entities", {-float_min, -float_min}))
  {
    const auto scene = scene_.lock();
    if (scene)
    {
      const auto &registry = scene->registry();
      const auto  view     = registry.view<NameComponent>();

      for (const auto &entity_handle : view)
      {
        Entity entity{entity_handle, scene_};

        const auto is_selected =
            selected_entity_.entity_handle() == entity_handle;

        const auto text = entity.name();
        if (ImGui::Selectable(text.c_str(), is_selected))
        {
          selected_entity_ = entity;
          const auto event =
              std::make_shared<EntitySelectedEvent>(selected_entity_);
          Engine::instance()->event_manager()->publish(event);
        }

        if (is_selected)
        {
          ImGui::SetItemDefaultFocus();
        }
      }
    }
    ImGui::EndListBox();
  }
}

bool ScenePanel::on_event(const Event &event)
{
  const auto event_id = event.id();
  if (event_id == SceneLoadedEvent::id)
  {
    return on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }

  return false;
}

bool ScenePanel::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
  return false;
}
