#include "script_engine.hpp"

namespace dc
{

ScriptEngine::ScriptEngine() { init_mono(); }

ScriptEngine::~ScriptEngine()
{
  // Apparently according to
  // https://www.mono-project.com/docs/advanced/embedding/ we can't do
  // mono_jit_init in the same process after mono_jit_cleanup... so don't do
  // this mono_jit_cleanup(s_MonoDomain);
}

void ScriptEngine::init_mono() { domain_ = mono_jit_init("discite"); }

} // namespace dc
