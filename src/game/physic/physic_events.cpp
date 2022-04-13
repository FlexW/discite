#include "physic_events.hpp"
#include "event.hpp"

namespace dc
{

EventId EntityCollisionBeginEvent::id{0x5056ca29};

EntityCollisionBeginEvent::EntityCollisionBeginEvent(Entity collider,
                                                     Entity collidee)
    : Event{id},
      collider_{collider},
      collidee_{collidee}
{
}

EventId EntityCollisionEndEvent::id{0x02061561};

EntityCollisionEndEvent::EntityCollisionEndEvent(Entity collider,
                                                 Entity collidee)
    : Event{id},
      collider_{collider},
      collidee_{collidee}
{
}

EventId EntityTriggerBeginEvent::id{0xc31685b0};

EntityTriggerBeginEvent::EntityTriggerBeginEvent(Entity trigger, Entity other)
    : Event{id},
      trigger_{trigger},
      other_{other}
{
}

EventId EntityTriggerEndEvent::id{0x8c8872ca};

EntityTriggerEndEvent::EntityTriggerEndEvent(Entity trigger, Entity other)
    : Event{id},
      trigger_{trigger},
      other_{other}
{
}

} // namespace dc
