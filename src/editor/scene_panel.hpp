#pragma once

#include "entity.hpp"
#include "event.hpp"
#include "imgui_panel.hpp"
#include "scene.hpp"

#include <memory>

class EntitySelectedEvent : public Event
{
public:
  static EventId id;

  Entity entity_;

  EntitySelectedEvent(Entity entity);
};

class ScenePanel : public ImGuiPanel
{
public:
  ScenePanel();

  bool on_event(const Event &event) override;

private:
  Entity               selected_entity_;
  std::weak_ptr<Scene> scene_{};

  void on_render() override;

  bool on_scene_loaded(const SceneLoadedEvent &event);
};
