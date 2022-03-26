#include "script_registry.hpp"
#include "script_wrapper.hpp"

#define MONO_METHOD(m) reinterpret_cast<void *>(m)

namespace dc::script_registry
{

void register_all()
{
  // logging
  mono_add_internal_call("Dc.Log::LogMessage_Native",
                         MONO_METHOD(dc::script_wrapper::Dc_Log_LogMessage));
}

} // namespace dc::script_registry
