#pragma once

#include "component_types.hpp"
#include "entity.hpp"
#include "event.hpp"

#include <memory>

namespace dc
{

class Entity;
class Scene;

/**
 * Event gets fired after the scene has been loaded
 */
class SceneLoadedEvent : public Event
{
public:
  static EventId id;

  std::shared_ptr<Scene> scene_;

  SceneLoadedEvent(std::shared_ptr<Scene> scene);
};

/**
 * Event gets fired after the scene got unloaded
 */
class SceneUnloadedEvent : public Event
{
public:
  static EventId id;

  std::shared_ptr<Scene> scene_;

  SceneUnloadedEvent(std::shared_ptr<Scene> scene);
};

class ComponentConstructEvent : public Event
{
public:
  static EventId id;

  Entity        entity_;
  ComponentType component_type_;

  ComponentConstructEvent(Entity entity, ComponentType component_type);
};

class ComponentDestroyEvent : public Event
{
public:
  static EventId id;

  Entity        entity_;
  ComponentType component_type_;

  ComponentDestroyEvent(Entity entity, ComponentType component_type);
};

} // namespace dc
