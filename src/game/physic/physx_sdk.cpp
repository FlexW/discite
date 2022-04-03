#include "physx_sdk.hpp"
#include "assert.hpp"
#include "log.hpp"
#include "physic/physx_debugger.hpp"
#include "physic/physx_helper.hpp"

#include <PxPhysicsVersion.h>
#include <common/PxTolerancesScale.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxExtensionsAPI.h>
#include <foundation/PxAssert.h>

#include <memory>
#include <thread>

namespace dc
{

void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code,
                                       const char              *message,
                                       const char              *file,
                                       int                      line)
{
  const char *errorMessage{};

  switch (code)
  {
  case physx::PxErrorCode::eNO_ERROR:
    errorMessage = "No Error";
    break;
  case physx::PxErrorCode::eDEBUG_INFO:
    errorMessage = "Info";
    break;
  case physx::PxErrorCode::eDEBUG_WARNING:
    errorMessage = "Warning";
    break;
  case physx::PxErrorCode::eINVALID_PARAMETER:
    errorMessage = "Invalid Parameter";
    break;
  case physx::PxErrorCode::eINVALID_OPERATION:
    errorMessage = "Invalid Operation";
    break;
  case physx::PxErrorCode::eOUT_OF_MEMORY:
    errorMessage = "Out Of Memory";
    break;
  case physx::PxErrorCode::eINTERNAL_ERROR:
    errorMessage = "Internal Error";
    break;
  case physx::PxErrorCode::eABORT:
    errorMessage = "Abort";
    break;
  case physx::PxErrorCode::ePERF_WARNING:
    errorMessage = "Performance Warning";
    break;
  case physx::PxErrorCode::eMASK_ALL:
    errorMessage = "Unknown Error";
    break;
  }

  switch (code)
  {
  case physx::PxErrorCode::eNO_ERROR:
  case physx::PxErrorCode::eDEBUG_INFO:
    DC_LOG_INFO("[PhysX]: {}: {} at {} ({})",
                errorMessage,
                message,
                file,
                line);
    break;
  case physx::PxErrorCode::eDEBUG_WARNING:
  case physx::PxErrorCode::ePERF_WARNING:
    DC_LOG_WARN("[PhysX]: {}: {} at {} ({})",
                errorMessage,
                message,
                file,
                line);
    break;
  case physx::PxErrorCode::eINVALID_PARAMETER:
  case physx::PxErrorCode::eINVALID_OPERATION:
  case physx::PxErrorCode::eOUT_OF_MEMORY:
  case physx::PxErrorCode::eINTERNAL_ERROR:
    DC_LOG_ERROR("[PhysX]: {}: {} at {} ({})",
                 errorMessage,
                 message,
                 file,
                 line);
    break;
  case physx::PxErrorCode::eABORT:
  case physx::PxErrorCode::eMASK_ALL:
    DC_LOG_ERROR("[PhysX]: {}: {} at {} ({})",
                 errorMessage,
                 message,
                 file,
                 line);
    DC_FAIL("PhysX failed!");
    break;
  }
}

void PhysicsAssertHandler::operator()(const char *exp,
                                      const char *file,
                                      int         line,
                                      bool & /*ignore*/)
{
  DC_FAIL("[PhysX Error]: {}:{} - {}", file, line, exp);
}

PhysXSdk *PhysXSdk::get_instance()
{
  static std::unique_ptr<PhysXSdk> instance(new PhysXSdk);
  return instance.get();
}

PhysXSdk::PhysXSdk()
{

  foundation_ =
      PxCreateFoundation(PX_PHYSICS_VERSION, allocator_, error_callback_);
  DC_ASSERT(foundation_, "PxCreateFoundation failed.");

  physx::PxTolerancesScale scale = physx::PxTolerancesScale();
  scale.length                   = 1.0f;
  scale.speed                    = 100.0f;

  const auto debugger = PhysXDebugger::get_instance();
  debugger->init(*foundation_);
  const auto px_debugger = debugger->get_debugger();

  physics_ = PxCreatePhysics(PX_PHYSICS_VERSION,
                             *foundation_,
                             scale,
                             true,
                             px_debugger);

  PxInitExtensions(*physics_, px_debugger);

  cpu_dispatcher_ =
      physx::PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());

  physx::PxSetAssertHandler(assert_handler_);
}

PhysXSdk::~PhysXSdk()
{
  px_release(cpu_dispatcher_);
  PxCloseExtensions();
  const auto debugger = PhysXDebugger::get_instance();
  debugger->stop_debugging();
  px_release(physics_);
  debugger->shutdown();
  px_release(foundation_);
}

physx::PxFoundation *PhysXSdk::get_foundation() const { return foundation_; }

physx::PxPhysics *PhysXSdk::get_physics() const { return physics_; }

physx::PxCpuDispatcher *PhysXSdk::get_cpu_dispatcher() const
{
  return cpu_dispatcher_;
}

physx::PxDefaultAllocator *PhysXSdk::get_allocator() { return &allocator_; }

} // namespace dc
