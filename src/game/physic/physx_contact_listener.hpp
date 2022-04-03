#pragma once

#include <PxPhysicsAPI.h>
#include <PxSimulationEventCallback.h>

namespace dc
{

class PhysXContactListener : public physx::PxSimulationEventCallback
{
public:
  virtual void onConstraintBreak(physx::PxConstraintInfo *constraints,
                                 physx::PxU32             count) override;

  virtual void onWake(physx::PxActor **actors, physx::PxU32 count) override;

  virtual void onSleep(physx::PxActor **actors, physx::PxU32 count) override;

  virtual void onContact(const physx::PxContactPairHeader &pairHeader,
                         const physx::PxContactPair       *pairs,
                         physx::PxU32                      nbPairs) override;

  virtual void onTrigger(physx::PxTriggerPair *pairs,
                         physx::PxU32          count) override;

  virtual void onAdvance(const physx::PxRigidBody *const *bodyBuffer,
                         const physx::PxTransform        *poseBuffer,
                         const physx::PxU32               count) override;
};

} // namespace dc
