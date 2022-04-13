#include "physx_contact_listener.hpp"
#include "engine.hpp"
#include "physic_actor.hpp"
#include "physic_events.hpp"

#include <PxFiltering.h>
#include <PxSimulationEventCallback.h>

#include <cstdint>

namespace dc
{

void PhysXContactListener::onConstraintBreak(
    physx::PxConstraintInfo * /*constraints*/,
    physx::PxU32 /*count*/)
{
}

void PhysXContactListener::onWake(physx::PxActor ** /*actors*/,
                                  physx::PxU32 /*count*/)
{
}

void PhysXContactListener::onSleep(physx::PxActor ** /*actors*/,
                                   physx::PxU32 /*count*/)
{
}

void PhysXContactListener::onContact(
    const physx::PxContactPairHeader &pairHeader,
    const physx::PxContactPair       *pairs,
    physx::PxU32 /*nbPairs*/)
{
  const auto removed_actor_a =
      pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
  const auto removed_actor_b =
      pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

  if (removed_actor_a || removed_actor_b)
  {
    return;
  }

  const auto actor_a =
      static_cast<PhysicActor *>(pairHeader.actors[0]->userData);
  const auto actor_b =
      static_cast<PhysicActor *>(pairHeader.actors[1]->userData);

  const auto entity_a = actor_a->get_entity();
  const auto entity_b = actor_b->get_entity();

  if (!entity_a.valid() || !entity_b.valid())
  {
    return;
  }

  const auto event_manger = Engine::instance()->event_manager();
  if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
  {
    EntityCollisionBeginEvent event{entity_a, entity_b};
    event_manger->fire_event(event);
  }
  else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
  {
    EntityCollisionEndEvent event{entity_a, entity_b};
    event_manger->fire_event(event);
  }
}

void PhysXContactListener::onTrigger(physx::PxTriggerPair *pairs,
                                     physx::PxU32          count)
{
  for (std::uint32_t i = 0; i < count; ++i)
  {
    const auto pair = pairs[i];
    if (pair.flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER |
                      physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
    {
      continue;
    }

    const auto trigger_actor =
        static_cast<PhysicActor *>(pair.triggerActor->userData);
    const auto other_actor =
        static_cast<PhysicActor *>(pair.otherActor->userData);

    if (!trigger_actor || !other_actor)
    {
      continue;
    }

    auto trigger_entity = trigger_actor->get_entity();
    auto other_entity   = other_actor->get_entity();

    const auto event_manger = Engine::instance()->event_manager();
    if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
    {
      EntityTriggerBeginEvent event{trigger_entity, other_entity};
      event_manger->fire_event(event);
    }
    else if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
    {
      EntityTriggerEndEvent event{trigger_entity, other_entity};
      event_manger->fire_event(event);
    }
  }
}

void PhysXContactListener::onAdvance(
    const physx::PxRigidBody *const * /*bodyBuffer*/,
    const physx::PxTransform * /*poseBuffer*/,
    const physx::PxU32 /*count*/)
{
}

} // namespace dc
