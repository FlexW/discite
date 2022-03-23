#pragma once

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace dc
{

class ScriptEngine
{
public:
  ScriptEngine();
  ~ScriptEngine();

private:
  MonoDomain *domain_{};

  void init_mono();
  void shutdown_mono();
};

} // namespace dc
