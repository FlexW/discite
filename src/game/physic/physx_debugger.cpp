#include "physx_debugger.hpp"
#include "assert.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "physic/physx_helper.hpp"
#include "physic/physx_sdk.hpp"
#include "solver/PxSolverDefs.h"

#include <PxFoundation.h>
#include <pvd/PxPvd.h>
#include <pvd/PxPvdTransport.h>

#include <memory>

namespace dc
{

PhysXDebugger::~PhysXDebugger()
{
  DC_ASSERT(!debugger_, "Debugger was not shutdown");
}

void PhysXDebugger::init(physx::PxFoundation &foundation)
{
  DC_ASSERT(!is_init_, "Already init");
  is_init_ = true;

  debugger_ = physx::PxCreatePvd(foundation);
  DC_ASSERT(debugger_, "Could not create PhysX debugger");
}

void PhysXDebugger::shutdown()
{
  DC_ASSERT(is_init_, "Not init");
  px_release(debugger_);
  debugger_ = nullptr;
}

PhysXDebugger *PhysXDebugger::get_instance()
{
  static std::unique_ptr<PhysXDebugger> instance(new PhysXDebugger);
  return instance.get();
}

void PhysXDebugger::start_debugging(const std::string &path,
                                    bool               network_debugging)
{
  stop_debugging();

  if (!network_debugging)
  {
    const auto name = path + ".pxd2";
    transport_      = physx::PxDefaultPvdFileTransportCreate(name.c_str());
    debugger_->connect(*transport_, physx::PxPvdInstrumentationFlag::eALL);

    DC_LOG_INFO("Start file PhysX debugging on {}", name);
  }
  else
  {
    const char *host{"localhost"};
    const int   port{5425};

    transport_ = physx::PxDefaultPvdSocketTransportCreate(host, port, 1000);
    debugger_->connect(*transport_, physx::PxPvdInstrumentationFlag::eALL);

    DC_LOG_INFO("Start network PhysX debugging on {}:{}", host, port);
  }
}

void PhysXDebugger::stop_debugging()
{
  if (!is_debugging())
  {
    return;
  }

  DC_LOG_INFO("Stop PhysX debugging");

  debugger_->disconnect();
  px_release(transport_);
}

bool PhysXDebugger::is_debugging() const
{
  if (debugger_)
  {
    return debugger_->isConnected();
  }
  return false;
}

physx::PxPvd *PhysXDebugger::get_debugger() const { return debugger_; }

} // namespace dc
