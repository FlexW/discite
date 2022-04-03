#pragma once

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <foundation/PxErrorCallback.h>
#include <task/PxCpuDispatcher.h>

namespace dc
{

class PhysicsErrorCallback : public physx::PxErrorCallback
{
public:
  virtual void reportError(physx::PxErrorCode::Enum code,
                           const char              *message,
                           const char              *file,
                           int                      line) override;
};

class PhysicsAssertHandler : public physx::PxAssertHandler
{
  virtual void
  operator()(const char *exp, const char *file, int line, bool &ignore);
};

class PhysXSdk
{
public:
  static PhysXSdk *get_instance();

  ~PhysXSdk();

  physx::PxFoundation       *get_foundation() const;
  physx::PxPhysics          *get_physics() const;
  physx::PxCpuDispatcher    *get_cpu_dispatcher() const;
  physx::PxDefaultAllocator *get_allocator();

private:
  physx::PxFoundation           *foundation_{};
  physx::PxDefaultCpuDispatcher *cpu_dispatcher_{};
  physx::PxPhysics              *physics_{};

  physx::PxDefaultAllocator allocator_;
  PhysicsErrorCallback      error_callback_;
  PhysicsAssertHandler      assert_handler_;

  PhysXSdk();
};

} // namespace dc
