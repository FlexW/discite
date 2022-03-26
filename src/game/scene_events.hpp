#pragma once

#include "entity.hpp"
#include "event.hpp"

class Scene;

namespace dc
{

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

class ScriptComponentConstructEvent : public Event
{
public:
  static EventId id;

  Entity entity_;

  ScriptComponentConstructEvent(Entity entity);
};

class ScriptComponentDestroyEvent : public Event
{
public:
  static EventId id;

  Entity entity_;

  ScriptComponentDestroyEvent(Entity entity);
};

} // namespace dc
