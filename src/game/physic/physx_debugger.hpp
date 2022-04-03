#pragma once

#include <PxFoundation.h>
#include <pvd/PxPvd.h>

#include <filesystem>

namespace dc
{

class PhysXDebugger
{
public:
  ~PhysXDebugger();

  static PhysXDebugger *get_instance();

  void init(physx::PxFoundation &foundation);
  void shutdown();

  void start_debugging(const std::string &path, bool network_debugging = false);

  void stop_debugging();

  bool is_debugging() const;

  physx::PxPvd *get_debugger() const;

private:
  bool is_init_{false};

  physx::PxPvd          *debugger_{};
  physx::PxPvdTransport *transport_{};

  PhysXDebugger() = default;
};

} // namespace dc
