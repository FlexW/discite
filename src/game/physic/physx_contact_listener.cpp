#include "physx_contact_listener.hpp"

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
    const physx::PxContactPairHeader & /*pairHeader*/,
    const physx::PxContactPair * /*pairs*/,
    physx::PxU32 /*nbPairs*/)
{
}

void PhysXContactListener::onTrigger(physx::PxTriggerPair * /*pairs*/,
                                     physx::PxU32 /*count*/)
{
}

void PhysXContactListener::onAdvance(
    const physx::PxRigidBody *const * /*bodyBuffer*/,
    const physx::PxTransform * /*poseBuffer*/,
    const physx::PxU32 /*count*/)
{
}

} // namespace dc
