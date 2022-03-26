#include "scene_events.hpp"

namespace dc
{

EventId SceneLoadedEvent::id{0x87a8e8c9};

SceneLoadedEvent::SceneLoadedEvent(std::shared_ptr<Scene> scene)
    : Event{id},
      scene_{scene}
{
}

EventId SceneUnloadedEvent::id{0xa26517c8};

SceneUnloadedEvent::SceneUnloadedEvent(std::shared_ptr<Scene> scene)
    : Event{id},
      scene_{scene}
{
}

EventId ScriptComponentConstructEvent::id{0x22bdd75f};

ScriptComponentConstructEvent::ScriptComponentConstructEvent(Entity entity)
    : Event{id},
      entity_{entity}
{
}

EventId ScriptComponentDestroyEvent::id{0x873c615f};

ScriptComponentDestroyEvent::ScriptComponentDestroyEvent(Entity entity)
    : Event{id},
      entity_{entity}
{
}

} // namespace dc
