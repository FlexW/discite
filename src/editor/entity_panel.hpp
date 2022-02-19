#pragma once

#include "event.hpp"
#include "imgui_panel.hpp"
#include "scene.hpp"
#include "scene_panel.hpp"

#include <memory>

class EntityPanel : public ImGuiPanel
{
public:
  EntityPanel();

  bool on_event(const Event &event) override;

private:
  std::weak_ptr<Scene> scene_{};
  Entity entity_;

  void on_render() override;

  bool on_entity_selected(const EntitySelectedEvent &event);
  bool on_scene_unloaded(const SceneUnloadedEvent &event);
};
