#pragma once

#include "entity.hpp"
#include "event.hpp"
namespace dc
{

class EntityCollisionBeginEvent : public Event
{
public:
  static EventId id;

  Entity collider_;
  Entity collidee_;

  EntityCollisionBeginEvent(Entity collider, Entity collidee);
};

class EntityCollisionEndEvent : public Event
{
public:
  static EventId id;

  Entity collider_;
  Entity collidee_;

  EntityCollisionEndEvent(Entity collider, Entity collidee);
};

class EntityTriggerBeginEvent : public Event
{
public:
  static EventId id;

  Entity trigger_;
  Entity other_;

  EntityTriggerBeginEvent(Entity trigger, Entity other);
};

class EntityTriggerEndEvent : public Event
{
public:
  static EventId id;

  Entity trigger_;
  Entity other_;

  EntityTriggerEndEvent(Entity trigger, Entity other);
};

} // namespace dc
