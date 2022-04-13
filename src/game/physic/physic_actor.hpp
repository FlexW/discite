#pragma once

#include "entity.hpp"
namespace dc
{

enum class PhysicActorType
{
  RigidBody,
  CharacterController,
};

class PhysicActor
{
public:
  virtual ~PhysicActor() = default;

  Entity get_entity() const { return entity_; }

  PhysicActorType physic_actor_type() const { return physic_actor_type_; }

protected:
  PhysicActor(Entity entity, PhysicActorType physic_actor_type)
      : entity_{entity},
        physic_actor_type_{physic_actor_type}
  {
  }

private:
  Entity          entity_;
  PhysicActorType physic_actor_type_;
};

} // namespace dc
