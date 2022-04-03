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

EventId ComponentConstructEvent::id{0x22bdd75f};

ComponentConstructEvent::ComponentConstructEvent(Entity        entity,
                                                 ComponentType component_type)
    : Event{id},
      entity_{entity},
      component_type_{component_type}
{
}

EventId ComponentDestroyEvent::id{0x873c615f};

ComponentDestroyEvent::ComponentDestroyEvent(Entity        entity,
                                             ComponentType component_type)
    : Event{id},
      entity_{entity},
      component_type_{component_type}
{
}

} // namespace dc
