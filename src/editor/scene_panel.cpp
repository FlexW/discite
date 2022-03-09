#include "scene_panel.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "entt/core/fwd.hpp"
#include "event.hpp"
#include "imgui.h"
#include "imgui_panel.hpp"
#include "name_component.hpp"
#include "relationship_component.hpp"
#include "scene.hpp"

#include <limits>
#include <memory>
#include <string>

namespace dc
{

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

  const auto scene = scene_.lock();
  if (scene)
  {
    const auto &view =
        scene->all_entities_with<NameComponent, RelationshipComponent>();
    for (const auto &e : view)
    {
      Entity entity{e, scene};
      if (!entity.has_parent())
      {
        draw_entity_node(entity);
      }
    }
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

void ScenePanel::draw_entity_node(Entity entity)
{
  ImGuiTreeNodeFlags flags{ImGuiTreeNodeFlags_OpenOnArrow |
                           ImGuiTreeNodeFlags_SpanAvailWidth};
  if (selected_entity_ == entity)
  {
    flags |= ImGuiTreeNodeFlags_Selected;
  }
  if (!entity.has_childs())
  {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  const auto id = static_cast<entt::id_type>(entity.entity_handle());

  const auto opened = ImGui::TreeNodeEx(reinterpret_cast<void *>(id),
                                        flags,
                                        "%s",
                                        entity.name().c_str());

  if (ImGui::IsItemClicked())
  {
    selected_entity_ = entity;
    const auto event = std::make_shared<EntitySelectedEvent>(selected_entity_);
    Engine::instance()->event_manager()->publish(event);
  }

  if (opened)
  {
    auto child = entity.component<RelationshipComponent>().first_child_;
    while (child.valid())
    {
      draw_entity_node(child);
      child = child.component<RelationshipComponent>().next_sibling_;
    }

    ImGui::TreePop();
  }
}

} // namespace dc
