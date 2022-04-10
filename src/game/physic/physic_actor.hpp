#pragma once

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

  PhysicActorType physic_actor_type() const { return physic_actor_type_; }

protected:
  PhysicActor(PhysicActorType physic_actor_type)
      : physic_actor_type_{physic_actor_type}
  {
  }

private:
  PhysicActorType physic_actor_type_;
};

} // namespace dc
